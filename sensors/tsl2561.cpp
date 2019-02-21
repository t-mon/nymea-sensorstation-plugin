/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                 *
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

#include "tsl2561.h"
#include "i2cport.h"
#include "sensordatafilter.h"
#include "extern-plugininfo.h"

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/i2c-dev.h>

#include <QFile>

TSL2561::TSL2561(const QString &i2cPortName, int i2cAddress, QObject *parent) :
    QThread(parent),
    m_i2cPortName(i2cPortName),
    m_i2cAddress(i2cAddress)
{

}

TSL2561::~TSL2561()
{
    disable();
    wait();
}

double TSL2561::currentLux()
{
    QMutexLocker valueLocker(&m_valueMutex);
    return m_currentLux;
}

void TSL2561::run()
{
    qCDebug(dcAnalogSensors()) << "TSL2561: initialize I2C port" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
    QFile i2cFile("/dev/" + m_i2cPortName);
    if (!i2cFile.exists()) {
        qCWarning(dcAnalogSensors()) << "TSL2561: The given I2C file descriptor does not exist:" << i2cFile.fileName();
        return;
    }

    if (!i2cFile.open(QFile::ReadWrite)) {
        qCWarning(dcAnalogSensors()) << "TSL2561: Could not open the given I2C file descriptor:" << i2cFile.fileName() << i2cFile.errorString();
        return;
    }

    m_fileDescriptor = i2cFile.handle();

    // Power up sensor and configure timing (402ms)
    if (!setPower(true) || !setTiming())
        return;

    // Data filer for smoothing sensor values
    SensorDataFilter luxFilter(SensorDataFilter::TypeLowPass);
    luxFilter.setFilterWindowSize(10);
    luxFilter.setLowPassAlpha(0.3);

    // Continuouse reading of the ADC values
    qCDebug(dcAnalogSensors()) << "TSL2561: start reading value thread..." << this << "Process PID:" << syscall(SYS_gettid);
    while (true) {
        if (ioctl(m_fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
            qCWarning(dcAnalogSensors()) << "TSL2561: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
            msleep(500);
            continue;
        }

        // Prepare reading
        quint8 reg[1] = {0x8C};
        if (write(m_fileDescriptor, reg, 1) != 1) {
            qCWarning(dcAnalogSensors()) << "TSL2561: could configure sensor for reading.";
            msleep(500);
            continue;
        }

        // Read data
        quint8 data[4] = {0};
        if (read(m_fileDescriptor, data, 4) != 4) {
            qCWarning(dcAnalogSensors()) << "TSL2561: could not configure sensor for reading.";
            msleep(500);
            continue;
        }

        // Note: convert to big endian
        quint16 channel0 = static_cast<quint16>((data[1] << 8) | data[0]);
        quint16 channel1 = static_cast<quint16>((data[3] << 8) | data[2]);
        quint16 visibleLight = channel0 - channel1;

        // Set the visible light as current value
        QMutexLocker valueLocker(&m_valueMutex);
        m_currentLux = qRound(luxFilter.filterValue(static_cast<double>(visibleLight)));
        //qCDebug(dcAnalogSensors()) << "Full spectrum:" << channel0 << "[lux] | Infrared:" << channel1 << "[lux] | Visible:" << m_currentLux << "[lux]";

        QMutexLocker stopLocker(&m_stopMutex);
        if (m_stop) break;
        msleep(500);
    }

    setPower(false);

    i2cFile.close();
    m_fileDescriptor = -1;
    qCDebug(dcAnalogSensors()) << "TSL2561: Reading thread finished.";
}

bool TSL2561::setPower(bool power)
{
    if (ioctl(m_fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
        qCWarning(dcAnalogSensors()) << "TSL2561: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
        return false;
    }

    // Power on 0x03, power off 0x00
    quint8 config[2] = {0};
    config[0] = 0x80;
    config[1] = (power ? 0x03 : 0x00);
    if (write(m_fileDescriptor, config, 2) != 2) {
        qCWarning(dcAnalogSensors()) << "TSL2561: Could not power" << (power ? "on" : "off") << "sensor.";
        return false;
    }
    return true;
}

bool TSL2561::setTiming()
{
    if (ioctl(m_fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
        qCWarning(dcAnalogSensors()) << "TSL2561: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
        return false;
    }

    // Integration time = 402ms(0x02)
    quint8 config[2] = {0};
    config[0] = 0x81;
    config[1] = 0x02;
    if (write(m_fileDescriptor, config, 2) != 2) {
        qCWarning(dcAnalogSensors()) << "TSL2561: Could not configure timings for sensor.";
        return false;
    }
    return true;
}

bool TSL2561::enable()
{
    // Check if this address can be opened
    I2CPort port(m_i2cPortName);
    if (!port.openPort(m_i2cAddress)) {
        qCWarning(dcAnalogSensors()) << "TSL2561 is not available on port" << port.portDeviceName() << QString("0x%1").arg(m_i2cAddress, 0, 16);;
        return false;
    }
    port.closePort();

    // Start the reading thread
    QMutexLocker locker(&m_stopMutex);
    m_stop = false;
    start();
    return true;
}

void TSL2561::disable()
{
    // Stop the thread if not already disabled
    QMutexLocker locker(&m_stopMutex);
    if (m_stop)
        return;

    qCDebug(dcAnalogSensors()) << "TSL2561: Disable measurements";
    m_stop = true;
}
