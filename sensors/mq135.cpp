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

#include "mq135.h"
#include "math.h"

/// The load resistance on the board
#define RLOAD 10.0

/// Calibration resistance at atmospheric CO2 level
#define RZERO 350 //76.63

/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 397.13

MQ135::MQ135(QObject *parent) : QObject(parent)
{
    m_filter = new SensorDataFilter(SensorDataFilter::TypeLowPass, this);
    m_filter->setLowPassAlpha(0.4);
    m_filter->setFilterWindowSize(5);
}

void MQ135::setAdcValue(int adcValue)
{
    m_adcValue = adcValue;
}

void MQ135::setTemperature(double temperature)
{
    m_temperature = temperature;
}

void MQ135::setHumidity(double humidity)
{
    m_humidity = humidity;
}

double MQ135::calculatePpmValue()
{
    return qRound(m_filter->filterValue(PARA * pow((getResistance() / RZERO), -PARB)));
}

double MQ135::getCalibrationRestistance()
{
    return getRZero();
}

double MQ135::getCorrectionFactor()
{
    return CORA * m_temperature * m_temperature - CORB * m_temperature + CORC - (m_humidity - 33.0) * CORD;
}

double MQ135::getResistance()
{
    // Note: 32767 is the max value of the the ADC with gain 1
    return ((32767.0 * 4.096 / m_adcValue) - 1.0) * RLOAD;
}

double MQ135::getCorrectedResistance()
{
    return getResistance() / getCorrectionFactor();
}

double MQ135::getCorrectedPPM()
{
    return PARA * pow((getCorrectedResistance() / RZERO), -PARB);
}

double MQ135::getRZero()
{
    return getResistance() * pow((ATMOCO2 / PARA), (1.0 / PARB));
}

double MQ135::getCorrectedRZero()
{
    return getCorrectedResistance() * pow((ATMOCO2 / PARA), (1.0 / PARB));
}
