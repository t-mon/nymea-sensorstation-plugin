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

#ifndef MQ135_H
#define MQ135_H

#include <QObject>

#include "sensordatafilter.h"

// Reference: https://github.com/GeorgK/MQ135

class MQ135 : public QObject
{
    Q_OBJECT
public:
    explicit MQ135(QObject *parent = nullptr);

    void setAdcValue(int adcValue);
    void setTemperature(double temperature);
    void setHumidity(double humidity);

    double calculatePpmValue();
    double getCalibrationRestistance();

private:
    SensorDataFilter *m_filter = nullptr;

    int m_adcValue = 0;
    double m_temperature = 22.0;
    double m_humidity = 50.0;

    double getCorrectionFactor();
    double getResistance();
    double getCorrectedResistance();
    double getCorrectedPPM();
    double getRZero();
    double getCorrectedRZero();

};

#endif // MQ135_H
