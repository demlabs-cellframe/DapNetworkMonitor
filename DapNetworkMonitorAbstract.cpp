#include "DapNetworkMonitorAbstract.h"


DapNetworkMonitorAbstract::DapNetworkMonitorAbstract(QObject *parent)
    : QObject(parent)
{
    connect(this, &DapNetworkMonitorAbstract::sigMonitoringStarted, [this]{
        m_isMonitoringRunning.store(true);
    });

    connect(this, &DapNetworkMonitorAbstract::sigMonitoringFinished, [this]{
        m_isMonitoringRunning.store(false);
    });
}

    void DapNetworkMonitorAbstract::interfaceUndefined()
    {
        emit sigInterfaceUndefined();
    }

    void DapNetworkMonitorAbstract::interfaceDefined()
    {
        emit sigInterfaceDefined();
    }
