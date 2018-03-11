include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginanalogsensors)

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginanalogsensors.cpp \
    airqualitysensor.cpp \
    interintegratedcircuit.cpp \
    interintegratedcircuitport.cpp

HEADERS += \
    devicepluginanalogsensors.h \
    airqualitysensor.h \
    interintegratedcircuit.h \
    interintegratedcircuitport.h

