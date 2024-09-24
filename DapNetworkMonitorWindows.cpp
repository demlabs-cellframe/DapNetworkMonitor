#include <QThread>

#include "DapNetworkMonitorWindows.h"

DapNetworkMonitorWindows::DapNetworkMonitorWindows(QObject *parent): DapNetworkMonitorAbstract(parent)
{
    qInfo() << "Dap Network Monitor started";
    QtConcurrent::run(this, &DapNetworkMonitorWindows::internalWorker);
}

bool DapNetworkMonitorWindows::isTunDriverInstalled() const
{
    return (getTapGUID() != NULL);
}

bool DapNetworkMonitorWindows::monitoringStart()
{
    m_isMonitoringRunning.store(true);
    return m_isMonitoringRunning;
}

bool DapNetworkMonitorWindows::monitoringStop()
{
    m_isMonitoringRunning.store(false);
    return m_isMonitoringRunning;
}

void DapNetworkMonitorWindows::cbRouteChanged(void *, PMIB_IPFORWARD_ROW2 route, MIB_NOTIFICATION_TYPE type)
{
    if (!instance()->m_isMonitoringRunning.load())
    {
        return;
    }
    switch (type)
    {
    case MibAddInstance:
        if (route->NextHop.Ipv4.sin_addr.S_un.S_addr == 0)
        {
            if (route->InterfaceIndex == instance()->m_TapAdapterIndex ||
                    route->InterfaceIndex == instance()->m_DefaultAdapterIndex)
            {
                instance()->m_isTunGatewayDefined.store(true);
                emit instance()->sigTunGatewayDefined();
            }
            qInfo() << "Set default gw on interface [ " << route->InterfaceIndex << " ]";
        }
        break;
    case MibDeleteInstance:
        if (route->NextHop.Ipv4.sin_addr.S_un.S_addr == 0)
        {
            if (route->InterfaceIndex == instance()->m_TapAdapterIndex ||
                    route->InterfaceIndex == instance()->m_DefaultAdapterIndex)
            {
                instance()->m_isTunGatewayDefined.store(false);
                emit instance()->sigTunGatewayUndefined();
            }
            qInfo() << "Removed default gw on interface [ " << route->InterfaceIndex << " ]";
        }
        break;
    case MibParameterNotification:
        qInfo() << "Changed gw " << inet_ntoa(route->NextHop.Ipv4.sin_addr) <<
                   " on inteface [ " << route->InterfaceIndex << " ]";
        break;
    default:
        break;
    }
}

void DapNetworkMonitorWindows::cbIfaceChanged(void *, PMIB_IPINTERFACE_ROW row, MIB_NOTIFICATION_TYPE type)
{
    if (!instance()->m_isMonitoringRunning.load())
    {
        return;
    }
    switch (type)
    {
    case MibAddInstance:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] enabled";
        if (row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == instance()->m_DefaultAdapterIndex)
        {
            instance()->m_isInterfaceDefined.store(true);
            emit instance()->sigInterfaceDefined();
        }
        break;
    case MibDeleteInstance:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] disabled";
        if (row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == instance()->m_DefaultAdapterIndex)
        {
            instance()->m_isInterfaceDefined.store(false);
            emit instance()->interfaceUndefined();
        }
        break;
    /*case MibParameterNotification:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] settings changed";
        if (row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == instance()->m_DefaultAdapterIndex) {
            if (row->Connected) {
                qWarning() << "[ " << row->InterfaceIndex << " ] enabled";
                emit instance()->sigInterfaceDefined();
            } else {
                qWarning() << "[ " << row->InterfaceIndex << " ] disabled";
                emit instance()->sigInterfaceUndefined();
            }
        }
        break;
        */
    default:
        break;
    }
}

void DapNetworkMonitorWindows::internalWorker()
{
    HANDLE hAddrChange, hRouteChange;
    unsigned int ctx = 0;

    DWORD res = NotifyRouteChange2(AF_INET, cbRouteChanged, &ctx, TRUE, &hRouteChange);
    if (res != NO_ERROR) {
        qCritical() << "Can't trace route table, error " << res;
        return;
    }
    res = NotifyIpInterfaceChange(AF_INET, cbIfaceChanged, &ctx, TRUE, &hAddrChange);
    if (res != NO_ERROR) {
        qCritical() << "Can't trace network interfaces, error " << res;
        return;
    }
}

void DapNetworkMonitorWindows::procErr(const int a_err, const QString &a_str)
{
    Q_UNUSED(a_str)
    switch (a_err)
    {
    case WSAEACCES:
    case WSAENETUNREACH:
    case WSAEHOSTUNREACH:
        m_isHostReachable.store(false);
        break;
    default:
        break;
    }
}
