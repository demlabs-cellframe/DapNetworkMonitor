#include "DapNetworkMonitorAndroid.h"

DapNetworkMonitorAndroid::DapNetworkMonitorAndroid(QObject *parent):
    DapNetworkMonitorAbstract(parent)
{
    m_isTunGatewayDefined.store(true);
    m_isInterfaceDefined.store(true);
    m_isHostReachable.store(true);
}

bool DapNetworkMonitorAndroid::isTunDriverInstalled() const
{
    // TODO
    return true;
}

bool DapNetworkMonitorAndroid::monitoringStart()
{
    // TODO
    // Add checking all needed parameters for successful monitoring
    return false;
}
bool DapNetworkMonitorAndroid::monitoringStop()
{
    // TODO
    return false;
}
