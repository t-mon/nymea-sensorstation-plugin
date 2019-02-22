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

#ifndef AIRQUALITYMONITOR_H
#define AIRQUALITYMONITOR_H

#include <QFile>
#include <QObject>

#include "plugin/device.h"
#include "sensors/mq135.h"
#include "sensors/ads1115.h"
#include "sensors/sht30.h"
#include "sensors/bmp180.h"
#include "sensors/tsl2561.h"

#include "sensordatafilter.h"

class AirQualityMonitor : public QObject
{
    Q_OBJECT
public:
    explicit AirQualityMonitor(Device *device, QObject *parent = nullptr);
    ~AirQualityMonitor();

    Device *device() const;

private:
    Device *m_device = nullptr;
    bool m_writeLogs = false;

    ADS1115 *m_adc = nullptr;

    MQ135 *m_airQualitySensor = nullptr;
    SensorDataFilter *m_airQualityFilter = nullptr;

    SHT30 *m_temperatureHumiditySensor = nullptr;
    SensorDataFilter *m_temperatureFilter = nullptr;
    SensorDataFilter *m_humidityFilter = nullptr;

    BMP180 *m_pressureSensor = nullptr;
    SensorDataFilter *m_pressureFilter = nullptr;

    TSL2561 *m_lightSensor = nullptr;
    SensorDataFilter *m_lightFilter = nullptr;

    QFile *m_logfile = nullptr;

    double roundValue(double value);

public slots:
    void enable();
    void disable();
    void measure();

};

#endif // AIRQUALITYMONITOR_H
