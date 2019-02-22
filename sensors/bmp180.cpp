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

#include "bmp180.h"
#include "i2cport.h"
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
#include <QtEndian>

BMP180::BMP180(const QString &i2cPortName, int i2cAddress, QObject *parent) :
    QThread(parent),
    m_i2cPortName(i2cPortName),
    m_i2cAddress(i2cAddress)
{

}

BMP180::~BMP180()
{
    disable();
    wait();
}

double BMP180::currentPressureValue()
{
    QMutexLocker valueLocker(&m_valueMutex);
    return m_pressure;
}

double BMP180::currentAltitudeValue()
{
    QMutexLocker valueLocker(&m_valueMutex);
    return m_altitude;
}

void BMP180::run()
{
    QFile i2cFile("/dev/" + m_i2cPortName);
    qCDebug(dcSensorStation()) << "BMP180: initialize I2C port" << i2cFile.fileName() << QString("0x%1").arg(m_i2cAddress, 0, 16);
    if (!i2cFile.exists()) {
        qCWarning(dcSensorStation()) << "BMP180: The given I2C file descriptor does not exist:" << i2cFile.fileName();
        return;
    }

    if (!i2cFile.open(QFile::ReadWrite)) {
        qCWarning(dcSensorStation()) << "BMP180: Could not open the given I2C file descriptor:" << i2cFile.fileName() << i2cFile.errorString();
        return;
    }

    int fileDescriptor = i2cFile.handle();
    qCDebug(dcSensorStation()) << "BMP180: start reading calibration values...";
    if (ioctl(fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
        qCWarning(dcSensorStation()) << "BMP180: Could not set I2C into slave mode" << i2cFile.fileName() << QString("0x%1").arg(m_i2cAddress, 0, 16);
        return;
    }

    // Load the calibration data from the sensors EEPROM
    loadCalibrationData(fileDescriptor);

    // Continuouse reading of the ADC values
    qCDebug(dcSensorStation()) << "BMP180: start measuring..." << this << "Process PID:" << syscall(SYS_gettid);
    while (true) {
        if (ioctl(fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
            qCWarning(dcSensorStation()) << "BMP180: Could not set I2C into slave mode" << i2cFile.fileName() << QString("0x%1").arg(m_i2cAddress, 0, 16);
            continue;
        }

        // Read temperature
        long rawTemperature = readRawTemperature(fileDescriptor);

        // Read pressure
        long rawPressure = readRawPressure(fileDescriptor);
        long pressure = calculatePressure(rawTemperature, rawPressure);
        double pressureConverted = pressure * 0.01;
        double altitude = calculateAltitude(pressure);

        // For debugging
        //double temperature = calculateTemperature(rawTemperature);
        //qCDebug(dcSensorStation()) << "BMP180: Temperature" << temperature <<  "[°C] | Pressure" << pressure << "[Pa]" << pressureConverted << "[hPa ]" << altitude << "[m]";

        QMutexLocker valueLocker(&m_valueMutex);
        m_altitude = altitude;
        m_pressure = pressureConverted;

        QMutexLocker stopLocker(&m_stopMutex);
        if (m_stop) break;
        msleep(500);
    }

    i2cFile.close();
    qCDebug(dcSensorStation()) << "BMP180: Reading thread finished.";
}

void BMP180::loadCalibrationData(int fileDescriptor)
{
#ifdef __arm__
    m_calibrationAc1 = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xAA)));
    m_calibrationAc2 = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xAC)));
    m_calibrationAc3 = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xAE)));
    m_calibrationAc4 = qToBigEndian(static_cast<quint16>(i2c_smbus_read_word_data(fileDescriptor, 0xB0)));
    m_calibrationAc5 = qToBigEndian(static_cast<quint16>(i2c_smbus_read_word_data(fileDescriptor, 0xB2)));
    m_calibrationAc6 = qToBigEndian(static_cast<quint16>(i2c_smbus_read_word_data(fileDescriptor, 0xB4)));
    m_calibrationB1 = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xB6)));
    m_calibrationB2 = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xB8)));
    m_calibrationMB = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xBA)));
    m_calibrationMC = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xBC)));
    m_calibrationMD = qToBigEndian(static_cast<qint16>(i2c_smbus_read_word_data(fileDescriptor, 0xBE)));

    qCDebug(dcSensorStation()) << "BMP180: AC1" << m_calibrationAc1;
    qCDebug(dcSensorStation()) << "BMP180: AC2" << m_calibrationAc2;
    qCDebug(dcSensorStation()) << "BMP180: AC3" << m_calibrationAc3;
    qCDebug(dcSensorStation()) << "BMP180: AC4" << m_calibrationAc4;
    qCDebug(dcSensorStation()) << "BMP180: AC5" << m_calibrationAc5;
    qCDebug(dcSensorStation()) << "BMP180: AC6" << m_calibrationAc6;
    qCDebug(dcSensorStation()) << "BMP180: B1" << m_calibrationB1;
    qCDebug(dcSensorStation()) << "BMP180: B2" << m_calibrationB2;
    qCDebug(dcSensorStation()) << "BMP180: MB" << m_calibrationMB;
    qCDebug(dcSensorStation()) << "BMP180: MC" << m_calibrationMC;
    qCDebug(dcSensorStation()) << "BMP180: MD" << m_calibrationMD;
#else
    Q_UNUSED(fileDescriptor)
#endif // __arm__
}

bool BMP180::sendCommand(int fileDescriptor, quint8 command)
{
#ifdef __arm__
    if (ioctl(fileDescriptor, I2C_SLAVE, m_i2cAddress) < 0) {
        qCWarning(dcSensorStation()) << "BMP180: Could not set I2C into slave mode" << m_i2cPortName << QString("0x%1").arg(m_i2cAddress, 0, 16);
        return false;
    }

    // Write command (0xF4)
    int length = i2c_smbus_write_byte_data(fileDescriptor, 0xF4, command);
    if (length < 0) {
        qCWarning(dcSensorStation()) << "BMP180: Could not sent command" << QString("0x%1").arg(command, 0, 16) << "to I2C bus.";
        return false;
    }
    return true;
#else
    Q_UNUSED(fileDescriptor)
    Q_UNUSED(command)
    return false;
#endif // __arm__
}

long BMP180::readRawTemperature(int fileDescriptor)
{
    long rawTemperature = 0;
#ifdef __arm__
    // Send command to measure temperature (0x2E)
    if (!sendCommand(fileDescriptor, 0x2E)) {
        return rawTemperature;
    }
    msleep(5);
    qint16 rawValue = i2c_smbus_read_word_data(fileDescriptor, 0xF6);
    rawTemperature = static_cast<long>(qToBigEndian(rawValue));
#else
    Q_UNUSED(fileDescriptor)
#endif // __arm__
    return rawTemperature;
}

double BMP180::calculateTemperature(long rawTemperature)
{
    // This calculation was taken directly from the data sheet.
    long x1 = (((rawTemperature) - m_calibrationAc6) * m_calibrationAc5) >> 15;
    long x2 = (m_calibrationMC << 11) / (x1 + m_calibrationMD);
    long b5 = x1 + x2;
    return static_cast<double>((b5 + 8) >> 4) / 10.0;
}

long BMP180::readRawPressure(int fileDescriptor)
{
    long rawPressure = 0;
#ifdef __arm__
    // Send command to measure pressure (0x34)
    if (!sendCommand(fileDescriptor, 0x34 + (static_cast<quint8>(m_mode) << 6))) {
        return rawPressure;
    }

    switch (m_mode) {
    case OperationModeUltraLowPower:
        msleep(5);
        break;
    case OperationModeStandard:
        msleep(8);
        break;
    case OperationModeHighPower:
        msleep(14);
        break;
    case OperationModeUltraHighPower:
        msleep(26);
        break;
    }
    quint8 rawMsb = i2c_smbus_read_byte_data(fileDescriptor, 0xF6);
    quint8 rawLsb = i2c_smbus_read_byte_data(fileDescriptor, 0xF7);
    quint8 rawXlsb = i2c_smbus_read_byte_data(fileDescriptor, 0xF8);
    long msb = static_cast<long>(rawMsb);
    long lsb = static_cast<long>(rawLsb);
    long xlsb = static_cast<long>(rawXlsb);
    rawPressure = ((msb << 16) + (lsb << 8) + xlsb) >> (8 - static_cast<quint8>(m_mode));
#else
    Q_UNUSED(fileDescriptor)
#endif // __arm__
    return rawPressure;
}

long BMP180::calculatePressure(long rawTemperature, long rawPressure)
{
    // This calculation was taken directly from the data sheet.
    // Calculate B5
    long x1 = (((rawTemperature) - m_calibrationAc6) * m_calibrationAc5) >> 15;
    long x2 = (m_calibrationMC << 11) / (x1 + m_calibrationMD);
    long b5 = x1 + x2;
    //qCDebug(dcSensorStation()) << "   -> b5:" << b5;

    // Calculate B6
    long b6 = b5 - 4000;
    //qCDebug(dcSensorStation()) << "   -> b6:" << b6;

    // Calculate B3
    x1 = (m_calibrationB2 * (b6 * b6) >> 12 ) >> 11;
    x2 = (m_calibrationAc2 * b6) >> 11;
    long x3 = x1 + x2;
    long b3 = (((m_calibrationAc1 * 4 + x3) << static_cast<quint8>(m_mode)) + 2) / 4;
    //qCDebug(dcSensorStation()) << "   -> b3:" << b3;

    // Calculate B4
    x1 = (m_calibrationAc3 * b6) >> 13;
    x2 = (m_calibrationB1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) +2) >> 2;
    long b4 = (m_calibrationAc4 * (x3 + 32768)) >> 15;
    //qCDebug(dcSensorStation()) << "   -> b4:" << b4;

    // Calculate B7
    long b7 = (rawPressure - b3) * (50000 >> static_cast<long>(m_mode));
    //qCDebug(dcSensorStation()) << "   -> b7:" << b7;

    // calculate compensated pressure
    long p = 0;
    if (b7 < static_cast<long>(0x80000000)) {
        p = b7 * 2;
    } else {
        p = (b7 / b4) * 2;
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3083) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
    return p;
}

double BMP180::calculateAltitude(long pressure)
{
    // Calculate the altitude in [m] from the given pressure [Pa] (from the datasheet)
    // altitude = 44330.0 * (1.0 - pow(pressure / sealevel-pressure, (1.0/5.255)))
    return 44330.0 * (1.0 - pow(pressure / 101325.0, (1.0/5.255)));
}

bool BMP180::enable()
{
    // Check if this address can be opened
    I2CPort port(m_i2cPortName);
    if (!port.openPort(m_i2cAddress)) {
        qCWarning(dcSensorStation()) << "BMP180 is not available on port" << port.portDeviceName() << QString("0x%1").arg(m_i2cAddress, 0, 16);;
        return false;
    }
    port.closePort();

    // Start the reading thread
    QMutexLocker locker(&m_stopMutex);
    m_stop = false;
    start();
    return true;
}

void BMP180::disable()
{
    // Stop the thread if not already disabled
    QMutexLocker locker(&m_stopMutex);
    if (m_stop)
        return;

    qCDebug(dcSensorStation()) << "BMP180: Disable measurements";
    m_stop = true;
}

