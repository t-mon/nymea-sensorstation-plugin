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
#include "devicepluginanalogsensors.h"

#include "interintegratedcircuit.h"

DevicePluginAnalogSensors::DevicePluginAnalogSensors()
{

}

void DevicePluginAnalogSensors::init()
{
    // Initialize/create objects
}

void DevicePluginAnalogSensors::startMonitoringAutoDevices()
{
    // Start seaching for devices which can be discovered and added automatically
    qCDebug(dcAnalogSensors()) << "i2c ports:" << InterIntegratedCircuit::availablePorts();
}

void DevicePluginAnalogSensors::postSetupDevice(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Post setup device" << device->name() << device->params();

    // This method will be called once the setup for device is finished
}

void DevicePluginAnalogSensors::deviceRemoved(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Remove device" << device->name() << device->params();

    // Clean up all data related to this device
}

DeviceManager::DeviceSetupStatus DevicePluginAnalogSensors::setupDevice(Device *device)
{
    qCDebug(dcAnalogSensors()) << "Setup device" << device->name() << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginAnalogSensors::executeAction(Device *device, const Action &action)
{
    qCDebug(dcAnalogSensors()) << "Executing action for device" << device->name() << action.actionTypeId().toString() << action.params();

    return DeviceManager::DeviceErrorNoError;
}
