#include "DapNetworkMonitorAndroid.h"
#include <errno.h>

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

void DapNetworkMonitorAndroid::procErr(const int a_err, const QString &a_str)
{
    Q_UNUSED(a_str)
    switch (a_err)
    {
    case ENETUNREACH:
    case EHOSTUNREACH:
    case ENOLINK:
    case ENETDOWN:
        m_isHostReachable.store(false);
        break;
    default:
        break;
    }
}
