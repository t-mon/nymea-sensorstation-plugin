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

#ifndef ADS1115_H
#define ADS1115_H

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QMutexLocker>

class ADS1115 : public QThread
{
    Q_OBJECT
public:
    enum Channel {
        Channel1 = 0x00,
        Channel2 = 0x01,
        Channel3 = 0x02,
        Channel4 = 0x03
    };
    Q_ENUM(Channel)

    explicit ADS1115(const QString &i2cPortName, int i2cAddress = 0x48, QObject *parent = nullptr);
    ~ADS1115() override;

    double getChannelVoltage(Channel channel);
    int getChannelValue(Channel channel);

protected:
    void run() override;

private:
    QString m_i2cPortName;
    int m_i2cAddress;

    // Thread stuff
    QMutex m_stopMutex;
    bool m_stop = false;

    QMutex m_valueMutex;
    int m_channel1Value = 0;
    int m_channel2Value = 0;
    int m_channel3Value = 0;
    int m_channel4Value = 0;

    int readInputValue(int fd, Channel channel);

public slots:
    bool enable();
    void disable();

};

#endif // ADS1115_H
