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

#include "plugininfo.h"
#include "types/param.h"
#include "plugin/devicedescriptor.h"
#include "devicepluginanalogsensors.h"

DevicePluginAnalogSensors::DevicePluginAnalogSensors()
{

}

DevicePluginAnalogSensors::~DevicePluginAnalogSensors()
{
    if (m_timer) {
        hardwareManager()->pluginTimerManager()->unregisterTimer(m_timer);
        m_timer = nullptr;
    }
}

void DevicePluginAnalogSensors::init()
{

}

void DevicePluginAnalogSensors::postSetupDevice(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Post setup device" << device->name();
    if (m_airQualityMonitor) {
        m_airQualityMonitor->enable();
    }
}

void DevicePluginAnalogSensors::deviceRemoved(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Remove device" << device->name();

    // Clean up all data related to this device
    if (device->deviceClassId() == airQualitySensorsDeviceClassId) {
        if (m_airQualityMonitor) {
            delete m_airQualityMonitor;
            m_airQualityMonitor = nullptr;
        }

        if (m_timer) {
            hardwareManager()->pluginTimerManager()->unregisterTimer(m_timer);
            m_timer = nullptr;
        }
    }
}

DeviceManager::DeviceSetupStatus DevicePluginAnalogSensors::setupDevice(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Setup device" << device->name();

    if (device->deviceClassId() == airQualitySensorsDeviceClassId) {
        if (m_airQualityMonitor) {
            qCWarning(dcAnalogSensors()) << "There is already an analogsensor set up. Only once sensor is allowed";
            return DeviceManager::DeviceSetupStatusFailure;
        }

        m_airQualityMonitor = new AirQualityMonitor(device, this);
        m_timer = hardwareManager()->pluginTimerManager()->registerTimer(600);
        connect(m_timer, &PluginTimer::timeout, this, &DevicePluginAnalogSensors::onPluginTimer);
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginAnalogSensors::executeAction(Device *device, const Action &action)
{
    qCDebug(dcAnalogSensors()) << "Executing action for device" << device->name() << action.actionTypeId().toString() << action.params();

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginAnalogSensors::onPluginTimer()
{
    if (m_airQualityMonitor) {
        m_airQualityMonitor->measure();
    }
}
