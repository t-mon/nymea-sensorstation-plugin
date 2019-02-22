/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ads1115.h"
#include "i2cport.h"
#include "extern-plugininfo.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/i2c-dev.h>

#include <QFile>

ADS1115::ADS1115(const QString &i2cPortName, int i2cAddress, QObject *parent) :
    QThread(parent),
    m_i2cPortName(i2cPortName),
    m_i2cAddress(i2cAddress)
{

}

ADS1115::~ADS1115()
{
    disable();
    wait();
}

double ADS1115::getChannelVoltage(ADS1115::Channel channel)
{
    return static_cast<double>(getChannelValue(channel)) * 4.096 / 32767.0;
}

int ADS1115::getChannelValue(ADS1115::Channel channel)
{
    QMutexLocker locker(&m_valueMutex);
    int value = 0;
    switch (channel) {
    case Channel1:
        value = m_channel1Value;
        break;
    case Channel2:
        value =  m_channel2Value;
        break;
    case Channel3:
        value =  m_channel3Value;
        break;
    case Channel4:
        value =  m_channel4Value;
        break;
    }
    return value;
}

void ADS1115::run()
{
    qCDebug(dcSensorStation()) << "ADS1115: initialize I2C port" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);

    QFile i2cFile("/dev/" + m_i2cPortName);
    if (!i2cFile.exists()) {
        qCWarning(dcSensorStation()) << "ADS1115: The given I2C file descriptor does not exist:" << i2cFile.fileName();
        return;
    }

    if (!i2cFile.open(QFile::ReadWrite)) {
        qCWarning(dcSensorStation()) << "ADS1115: Could not open the given I2C file descriptor:" << i2cFile.fileName() << i2cFile.errorString();
        return;
    }

    int fileDescriptor = i2cFile.handle();

    // Continuouse reading of the ADC values
    qCDebug(dcSensorStation()) << "ADS1115: start reading values..." << this << "Process PID:" << syscall(SYS_gettid);
    while (true) {
        if (ioctl(fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
            qCWarning(dcSensorStation()) << "ADS1115: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
            msleep(500);
            continue;
        }

        QMutexLocker valueLocker(&m_valueMutex);
        m_channel1Value = readInputValue(fileDescriptor, Channel1);
        m_channel2Value = readInputValue(fileDescriptor, Channel2);
        m_channel3Value = readInputValue(fileDescriptor, Channel3);
        m_channel4Value = readInputValue(fileDescriptor, Channel4);

        //qCDebug(dcSensorStation()) << "AI0:" << m_channel1Value << "| AI1" << m_channel2Value << "| AI2" << m_channel3Value << "| AI3" << m_channel4Value;

        QMutexLocker stopLocker(&m_stopMutex);
        if (m_stop) break;
        msleep(500);
    }

    i2cFile.close();
    qCDebug(dcSensorStation()) << "ADS1115: Reading thread finished.";
}

int ADS1115::readInputValue(int fd, ADS1115::Channel channel)
{
    // AIN0 and GND, gain 1 = 4.096 V, 128 samples/s
    unsigned char writeBuf[3];
    writeBuf[0] = 0x01; // Config register
    switch (channel) {
    case Channel1:
        writeBuf[1] = 0xC3; // 0b11000011
        break;
    case Channel2:
        writeBuf[1] = 0xD3; // 0b11010011
        break;
    case Channel3:
        writeBuf[1] = 0xE3; // 0b11100011
        break;
    case Channel4:
        writeBuf[1] = 0xF3; // 0b11110011
        break;
    }
    writeBuf[2] = 0x85; // 0b10000101
    write(fd, writeBuf, 3);

    // Wait for conversion complete
    unsigned char readBuf[2] = {0};
    do {
        if (read(fd, readBuf, 2) != 2) {
            qCWarning(dcSensorStation()) << "ADS1115: could not read ADC data";
            return 0;
        }
    } while (!(readBuf[0] & 0x80));

    // Write conversion register
    readBuf[0] = 0;
    if (write(fd, readBuf, 1) != 1) {
        qCWarning(dcSensorStation()) << "ADS1115: could not write select register";
        return 0;
    }

    // Read value data
    if (read(fd, readBuf, 2) != 2) {
        qCWarning(dcSensorStation()) << "ADS1115: could not read ADC data";
        return 0;
    }

    int value = static_cast<qint16>(readBuf[0]) * 256 + static_cast<qint16>(readBuf[1]);
    return value;
}

bool ADS1115::enable()
{
    // Check if this address can be opened
    I2CPort port(m_i2cPortName);
    if (!port.openPort(m_i2cAddress)) {
        qCWarning(dcSensorStation()) << "ADS1115 is not available on port" << port.portDeviceName() << QString("0x%1").arg(m_i2cAddress, 0, 16);
        return false;
    }
    port.closePort();

    // Start the reading thread
    QMutexLocker locker(&m_stopMutex);
    m_stop = false;
    start();
    return true;
}

void ADS1115::disable()
{
    // Stop the thread if not already disabled
    QMutexLocker locker(&m_stopMutex);
    if (m_stop)
        return;

    qCDebug(dcSensorStation()) << "ADS1115: Disable measurements";
    m_stop = true;
}

