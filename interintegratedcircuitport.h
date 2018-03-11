#ifndef INTERINTEGRATEDCIRCUITPORT_H
#define INTERINTEGRATEDCIRCUITPORT_H

#include <QObject>

class InterIntegratedCircuitPort : public QObject
{
    Q_OBJECT
public:
    explicit InterIntegratedCircuitPort(QObject *parent = nullptr);

signals:

public slots:
};

#endif // INTERINTEGRATEDCIRCUITPORT_H