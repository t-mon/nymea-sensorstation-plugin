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

#ifndef SHT30_H
#define SHT30_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QMutexLocker>

class SHT30 : public QThread
{
    Q_OBJECT
public:
    explicit SHT30(const QString &i2cPortName = "i2c-1", int i2cAddress = 0x44, QObject *parent = nullptr);
    ~SHT30() override;

    double currentTemperatureValue();
    double currentHumidityValue();

protected:
    void run() override;

private:
    QString m_i2cPortName;
    int m_i2cAddress;
    bool m_available = false;

    // Thread stuff
    QMutex m_stopMutex;
    bool m_stop = false;

    QMutex m_valueMutex;
    double m_temperature;
    double m_humidity;

public slots:
    bool enable();
    void disable();

};

#endif // SHT30_H
