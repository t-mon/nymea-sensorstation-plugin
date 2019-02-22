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

#include "airqualitymonitor.h"
#include "extern-plugininfo.h"

#include <QDateTime>
#include <QTextStream>

/*
    Once connected the I2C devices can be found using the i2cdetect command on port 1 of the Raspberry Pi.

    $ i2cdetect -y 1

         0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    00:          -- -- -- -- -- -- -- -- -- -- -- -- --
    10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    30: -- -- -- -- -- -- -- -- -- 39 -- -- -- -- -- --
    40: -- -- -- -- 44 -- -- -- 48 -- -- -- -- -- -- --
    50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    70: -- -- -- -- -- -- -- 77

    Note: following devices can be found on folowing addresses
        - 0x39 : The light sensor TSL2561
        - 0x44 : The temperature/humidity sensor SHT30
        - 0x48 : The ADC ADS 1115 (where the MQ-135 is connected to AI0)
        - 0x77 : The pressure sensor BMP180
*/

AirQualityMonitor::AirQualityMonitor(Device *device, QObject *parent) :
    QObject(parent),
    m_device(device)
{
    // Create I2C devices
    m_adc = new ADS1115("i2c-1", 0x48, this);
    m_lightSensor = new TSL2561("i2c-1", 0x39, this);
    m_pressureSensor = new BMP180("i2c-1", 0x77, this);
    m_temperatureHumiditySensor = new SHT30("i2c-1", 0x44, this);

    m_airQualityFilter = new SensorDataFilter(SensorDataFilter::TypeAverage, this);
    m_airQualityFilter->setFilterWindowSize(5);

    m_temperatureFilter = new SensorDataFilter(SensorDataFilter::TypeAverage, this);
    m_temperatureFilter->setFilterWindowSize(3);

    m_humidityFilter = new SensorDataFilter(SensorDataFilter::TypeAverage, this);
    m_humidityFilter->setFilterWindowSize(3);

    m_pressureFilter = new SensorDataFilter(SensorDataFilter::TypeLowPass, this);
    m_pressureFilter->setFilterWindowSize(5);

    m_lightFilter = new SensorDataFilter(SensorDataFilter::TypeAverage, this);
    m_lightFilter->setFilterWindowSize(3);

    // Create the MQ-135 class and enable the ADC reading
    m_airQualitySensor = new MQ135(this);

    // Note: for debugging, if we want to log the sensordata for plotting and filter tests
    m_logfile = new QFile("/tmp/sensordata.log");
}

AirQualityMonitor::~AirQualityMonitor()
{
    if (m_logfile->isOpen()) {
        m_logfile->close();
    }
}

Device *AirQualityMonitor::device() const
{
    return m_device;
}

double AirQualityMonitor::roundValue(double value)
{
    return qRound(value * 100) / 100.0;
}

void AirQualityMonitor::enable()
{
    // Enable the sensors
    m_adc->enable();
    m_temperatureHumiditySensor->enable();
    m_pressureSensor->enable();
    m_lightSensor->enable();

    // Make device available
    m_device->setStateValue(sensorStationConnectedStateTypeId, true);

    // Open the logfile
    if (!m_logfile->isOpen() && m_writeLogs) {
        if (!m_logfile->open(QFile::WriteOnly  | QFile::Append)) {
            qCWarning(dcSensorStation()) << "Could not open logfile" << m_logfile->errorString();
        }
    }
}

void AirQualityMonitor::disable()
{
    // Disable the sensors
    m_adc->disable();
    m_temperatureHumiditySensor->disable();
    m_pressureSensor->disable();
    m_lightSensor->disable();

    // Make device unavailable
    m_device->setStateValue(sensorStationConnectedStateTypeId, false);
}

void AirQualityMonitor::measure()
{
    // SHT30
    double currentTemperature = m_temperatureHumiditySensor->currentTemperatureValue();
    double currentTemperatureFiltered = m_temperatureFilter->filterValue(currentTemperature);

    double currentHumidity = m_temperatureHumiditySensor->currentHumidityValue();
    double currentHumidityFiltered = m_humidityFilter->filterValue(currentHumidity);

    // BMP180
    double currentPressure = m_pressureSensor->currentPressureValue();
    double currentPressureFiltered = m_pressureFilter->filterValue(currentPressure);

    // TSL2561
    double currentLux = m_lightSensor->currentLux();
    double currentLuxFiltered = m_lightFilter->filterValue(currentLux);

    // CO2 ppm
    m_airQualitySensor->setTemperature(currentTemperature);
    m_airQualitySensor->setHumidity(currentHumidity);
    m_airQualitySensor->setAdcValue(m_adc->getChannelValue(ADS1115::Channel1));
    double currentPpm = m_airQualitySensor->calculatePpmValue();
    double currentPpmFiltered = m_airQualityFilter->filterValue(currentPpm);

    qCDebug(dcSensorStation()) << "Air quality value" << m_adc->getChannelValue(ADS1115::Channel1) << m_airQualitySensor->getCalibrationRestistance() << "Ohm" << m_adc->getChannelVoltage(ADS1115::Channel1) << "V" << currentPpm << "ppm";
    qCDebug(dcSensorStation()) << "Temperature" << currentTemperature << "[°C]" << "| Humidity" << currentHumidity << "[%]";
    qCDebug(dcSensorStation()) << "Pressure" << currentPressure << "[hPa]";
    qCDebug(dcSensorStation()) << "Light intensity" << currentLux << "[lux]";

    m_device->setStateValue(sensorStationCo2StateTypeId, roundValue(currentPpmFiltered));
    m_device->setStateValue(sensorStationTemperatureStateTypeId, roundValue(currentTemperatureFiltered));
    m_device->setStateValue(sensorStationHumidityStateTypeId, roundValue(currentHumidityFiltered));
    m_device->setStateValue(sensorStationPressureStateTypeId, roundValue(currentPressureFiltered));
    m_device->setStateValue(sensorStationLightIntensityStateTypeId, roundValue(currentLuxFiltered));

    // Write logfile for filter verification
    if (m_logfile->isOpen()) {
        QTextStream textStream(m_logfile);
        textStream << QDateTime::currentDateTime().toTime_t() << " "
                   << currentTemperature << " " << currentTemperatureFiltered << " "
                   << currentHumidity << " " << currentHumidityFiltered << " "
                   << currentPressure << " " << currentPressureFiltered << " "
                   << currentLux << " " << currentLuxFiltered << " "
                   << currentPpm << " " << currentPpmFiltered << " "
                   << endl;
    }
}
