#ifndef INTERINTEGRATEDCIRCUIT_H
#define INTERINTEGRATEDCIRCUIT_H

#include <QObject>

class InterIntegratedCircuit : public QObject
{
    Q_OBJECT
public:
    explicit InterIntegratedCircuit(QObject *parent = nullptr);

    static QStringList availablePorts();

signals:

public slots:
};

#endif // INTERINTEGRATEDCIRCUIT_H
