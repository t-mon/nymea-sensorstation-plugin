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

#ifndef TSL2561_H
#define TSL2561_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QMutexLocker>

// Reference: https://github.com/ControlEverythingCommunity/TSL2561

class TSL2561 : public QThread
{
    Q_OBJECT
public:
    explicit TSL2561(const QString &i2cPortName = "i2c-1", int i2cAddress = 0x39, QObject *parent = nullptr);
    ~TSL2561() override;

    double currentLux();

protected:
    void run() override;

private:
    QString m_i2cPortName;
    int m_i2cAddress;
    bool m_available = false;
    int m_fileDescriptor = -1;

    // Thread stuff
    QMutex m_stopMutex;
    bool m_stop = false;

    QMutex m_valueMutex;
    double m_currentLux = 0;

    // Init methods
    bool setPower(bool power);
    bool setTiming();

public slots:
    bool enable();
    void disable();

};

#endif // TSL2561_H
