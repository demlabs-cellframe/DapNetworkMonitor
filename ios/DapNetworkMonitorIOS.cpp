#include "DapNetworkMonitorIOS.h"
#include <errno.h>

DapNetworkMonitorIOS::DapNetworkMonitorIOS(QObject *parent):
    DapNetworkMonitorAbstract(parent)
{
    m_isTunGatewayDefined.store(true);
    m_isInterfaceDefined.store(true);
    m_isHostReachable.store(true);
}

bool DapNetworkMonitorIOS::isTunDriverInstalled() const
{
    // TODO
    return true;
}

bool DapNetworkMonitorIOS::monitoringStart()
{
    // TODO
    // Add checking all needed parameters for successful monitoring
    return false;
}
bool DapNetworkMonitorIOS::monitoringStop()
{
    // TODO
    return false;
}

void DapNetworkMonitorIOS::procErr(const int a_err, const QString &a_str) {
    Q_UNUSED(a_str)
    switch (a_err) {
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
