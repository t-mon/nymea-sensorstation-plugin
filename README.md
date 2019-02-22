# Sensor station
--------------------------------

This [nymea](https://nymea.io) plugin contains the implementation of a DIY sensor station containing:

* temperature and humitidy sensor
* light intensity sensor
* pressure
* air quality

![SensorStation](https://github.com/t-mon/nymea-sensorstation-plugin/blob/master/docs/screenshots/sensorstation.jpg "Sensor station")

## Build the plugin

This plugin was designed to run on a Raspberry Pi. For building this plugin the `libnymea1-dev` package has to be installed. How to get the nymea repository can be found in [here](https://nymea.io/en/wiki/nymea/master/install/debian). If starting from scratch the already finished image from the [Quick setup Raspberry Pi](https://nymea.io/en/wiki/nymea/master/usage/quick-setup-raspberry-pi) can also be done.

Assuming you are logged in using `ssh` to the Raspberry Pi and the nymea repository is available you can build the plugin using following steps

    apt update
    apt upgrade
    apt install git libnymea1-dev

Get the source code and build the plugin:

    git clone https://github.com/t-mon/nymea-sensorstation-plugin.git
    cd nymea-sensorstation-plugin
    mkdir build
    cd build
    qmake ../
    make -j$(nproc)

Finally you can install the plugin and restart `nymead` to make it available to the system.

    sudo make install
    sudo systemctrl restart nymead


## Schematics

![SensorStation schematics](https://github.com/t-mon/nymea-sensorstation-plugin/blob/master/docs/screenshots/Screenshot_20190222_111717.png "Sensor station schematics")

## I²C registers


      i2cdetect -y 1
    
             0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
        00:          -- -- -- -- -- -- -- -- -- -- -- -- --
        10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
        20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
        30: -- -- -- -- -- -- -- -- -- 39 -- -- -- -- -- --
        40: -- -- -- -- 44 -- -- -- 48 -- -- -- -- -- -- --
        50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
        60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
        70: -- -- -- -- -- -- -- 77


Note: following devices can be found on folowing addresses:

* `0x39`: The light sensor TSL2561
* `0x44`: The temperature/humidity sensor SHT30
* `0x48`: The ADC ADS 1115 (where the MQ-135 is connected to AI0)
* `0x77`: The pressure sensor BMP180


