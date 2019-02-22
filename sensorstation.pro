include(/usr/include/nymea/plugin.pri)

TARGET = $$qtLibraryTarget(nymea_devicepluginsensorstation)

QT *= network

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

HEADERS += \
    devicepluginsensorstation.h \
    i2cport.h \
    i2cport_p.h \
    sensors/ads1115.h \
    airqualitymonitor.h \
    sensors/mq135.h \
    sensors/bmp180.h \
    sensors/sht30.h \
    sensors/tsl2561.h \
    sensordatafilter.h

SOURCES += \
    devicepluginsensorstation.cpp \
    i2cport.cpp \
    sensors/ads1115.cpp \
    airqualitymonitor.cpp \
    sensors/mq135.cpp \
    sensors/bmp180.cpp \
    sensors/sht30.cpp \
    sensors/tsl2561.cpp \
    sensordatafilter.cpp

