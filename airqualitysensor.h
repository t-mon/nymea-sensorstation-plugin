#ifndef AIRQUALITYSENSOR_H
#define AIRQUALITYSENSOR_H

#include <QObject>

class AirQualitySensor : public QObject
{
    Q_OBJECT
public:
    explicit AirQualitySensor(QObject *parent = nullptr);

signals:

public slots:
};

#endif // AIRQUALITYSENSOR_H