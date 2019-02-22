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

#include "sht30.h"
#include "i2cport.h"
#include "sensordatafilter.h"
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

SHT30::SHT30(const QString &i2cPortName, int i2cAddress, QObject *parent) :
    QThread(parent),
    m_i2cPortName(i2cPortName),
    m_i2cAddress(i2cAddress)
{

}

SHT30::~SHT30()
{
    disable();
    wait();
}

double SHT30::currentTemperatureValue()
{
    QMutexLocker valueLocker(&m_valueMutex);
    return m_temperature;
}

double SHT30::currentHumidityValue()
{
    QMutexLocker valueLocker(&m_valueMutex);
    return m_humidity;
}

void SHT30::run()
{
    qCDebug(dcSensorStation()) << "SHT30: initialize I2C port" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);

    QFile i2cFile("/dev/" + m_i2cPortName);
    if (!i2cFile.exists()) {
        qCWarning(dcSensorStation()) << "SHT30: The given I2C file descriptor does not exist:" << i2cFile.fileName();
        return;
    }

    if (!i2cFile.open(QFile::ReadWrite)) {
        qCWarning(dcSensorStation()) << "SHT30: Could not open the given I2C file descriptor:" << i2cFile.fileName() << i2cFile.errorString();
        return;
    }

    int fileDescriptor = i2cFile.handle();

    // Data filer for smoothing sensor values
    SensorDataFilter temperatureFilter(SensorDataFilter::TypeLowPass);
    temperatureFilter.setFilterWindowSize(10);
    temperatureFilter.setLowPassAlpha(0.3);

    SensorDataFilter humidityFilter(SensorDataFilter::TypeLowPass);
    humidityFilter.setFilterWindowSize(10);
    humidityFilter.setLowPassAlpha(0.3);


    // Continuouse reading of the ADC values
    qCDebug(dcSensorStation()) << "SHT30: start reading value thread..." << this << "Process PID:" << syscall(SYS_gettid);
    while (true) {
        if (ioctl(fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
            qCWarning(dcSensorStation()) << "SHT30: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
            continue;
        }
        //qCDebug(dcSensorStation()) << "SHT30: set I2C address" << QString("0x%1").arg(m_i2cAddress, 0, 16);

        // Send measurement command: 0x2C
        // High repeatability measurement: 0x06
        char config[2] = {0};
        config[0] = 0x2C;
        config[1] = 0x06;
        if (write(fileDescriptor, config, 2) != 2) {
            qCWarning(dcSensorStation()) << "SHT30: could not configure sensor.";
            msleep(500);
            continue;
        }
        msleep(500);

        // Read 6 bytes of data
        // Temperature msb, Temperature lsb, Temperature CRC, Humididty msb, Humidity lsb, Humidity CRC
        char data[6] = {0};
        if (read(fileDescriptor, data, 6) != 6) {
            qCWarning(dcSensorStation()) << "SHT30: could not read sensor values.";
            msleep(500);
            continue;
        }

        // Convert the data
        int temperatureRaw = (data[0] << 8) | data[1];
        double temperature = -45 + (175 * temperatureRaw / 65535.0);
        double humidity = 100 * ((data[3] << 8) | data[4]) / 65535.0;

        QMutexLocker valueLocker(&m_valueMutex);
        m_temperature = temperatureFilter.filterValue(temperature);
        m_humidity = humidityFilter.filterValue(humidity);
        //qCDebug(dcSensorStation()) << "Temperature" << m_temperature << "°C | humidity" << m_humidity << "%";

        QMutexLocker stopLocker(&m_stopMutex);
        if (m_stop) break;
        msleep(500);
    }

    i2cFile.close();
    qCDebug(dcSensorStation()) << "SHT30: Reading thread finished.";
}

bool SHT30::enable()
{
    // Check if this address can be opened
    I2CPort port(m_i2cPortName);
    if (!port.openPort(m_i2cAddress)) {
        qCWarning(dcSensorStation()) << "SHT30 is not available on port" << port.portDeviceName() << QString("0x%1").arg(m_i2cAddress, 0, 16);;
        return false;
    }
    port.closePort();

    // Start the reading thread
    QMutexLocker locker(&m_stopMutex);
    m_stop = false;
    start();
    return true;
}

void SHT30::disable()
{
    // Stop the thread if not already disabled
    QMutexLocker locker(&m_stopMutex);
    if (m_stop)
        return;

    qCDebug(dcSensorStation()) << "SHT30: Disable measurements";
    m_stop = true;
}
