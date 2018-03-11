#include "interintegratedcircuit.h"

#include <QDir>

InterIntegratedCircuit::InterIntegratedCircuit(QObject *parent) : QObject(parent)
{

}

QStringList InterIntegratedCircuit::availablePorts()
{
    return QDir("/sys/class/i2c-adapter/").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}
