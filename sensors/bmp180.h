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

#ifndef BMP180_H
#define BMP180_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QMutexLocker>

class BMP180 : public QThread
{
    Q_OBJECT
public:
    enum OperationMode {
        OperationModeUltraLowPower = 0x00,
        OperationModeStandard = 0x01,
        OperationModeHighPower = 0x02,
        OperationModeUltraHighPower = 0x03
    };
    Q_ENUM(OperationMode)

    explicit BMP180(const QString &i2cPortName = "i2c-1", int i2cAddress = 0x77, QObject *parent = nullptr);
    ~BMP180() override;

    double currentPressureValue();
    double currentAltitudeValue();

protected:
    void run() override;

private:
    QString m_i2cPortName;
    int m_i2cAddress;
    bool m_available = false;

    // Thread stuff
    QMutex m_stopMutex;
    bool m_stop = false;

    qint16 m_calibrationAc1 = 0;
    qint16 m_calibrationAc2 = 0;
    qint16 m_calibrationAc3 = 0;
    quint16 m_calibrationAc4 = 0;
    quint16 m_calibrationAc5 = 0;
    quint16 m_calibrationAc6 = 0;
    qint16 m_calibrationB1 = 0;
    qint16 m_calibrationB2 = 0;
    qint16 m_calibrationMB = 0;
    qint16 m_calibrationMC = 0;
    qint16 m_calibrationMD = 0;

    OperationMode m_mode = OperationModeStandard;

    QMutex m_valueMutex;
    double m_pressure = 0;
    double m_altitude = 0;

    // Read methods for the sensor
    void loadCalibrationData(int fileDescriptor);
    bool sendCommand(int fileDescriptor, quint8 command);

    // Temperature calculation
    long readRawTemperature(int fileDescriptor);
    double calculateTemperature(long rawTemperature);

    // Pressure calculation
    long readRawPressure(int fileDescriptor);
    long calculatePressure(long rawTemperature, long rawPressure);
    double calculateAltitude(long pressure);
    double convertPressureValue();

public slots:
    bool enable();
    void disable();

};

#endif // BMP180_H
