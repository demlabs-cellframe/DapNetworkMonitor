#include "DapNetworkMonitorAbstract.h"


DapNetworkMonitorAbstract::DapNetworkMonitorAbstract(QObject *parent)
    : QObject(parent)
{
    connect(this, &DapNetworkMonitorAbstract::sigMonitoringStarted, [=]{
        m_isMonitoringRunning.store(true);
    });

    connect(this, &DapNetworkMonitorAbstract::sigMonitoringFinished, [=]{
        m_isMonitoringRunning.store(false);
    });
}
