#include <QThread>

#include "DapNetworkMonitorWindows.h"

DapNetworkMonitorWindows::DapNetworkMonitorWindows(QObject *parent):
    DapNetworkMonitorAbstract(parent) {
    qInfo() << "Dap Network Monitor started";
     QtConcurrent::run(this, &DapNetworkMonitorWindows::internalWorker);
}

bool DapNetworkMonitorWindows::isTunDriverInstalled() const {
    return (getTapGUID() != NULL);
}

bool DapNetworkMonitorWindows::isTunGatewayDefined() const {
    return m_isTunGatewayDefined;
}

bool DapNetworkMonitorWindows::isOtherGatewayDefined() const {
    return m_isOtherGatewayDefined;
}

bool DapNetworkMonitorWindows::monitoringStart() {
    QMutexLocker lock(&mutex);
    m_isMonitoringRunning = true;
    return m_isMonitoringRunning;
}

bool DapNetworkMonitorWindows::monitoringStop() {
    QMutexLocker lock(&mutex);
    m_isMonitoringRunning = false;
    return m_isMonitoringRunning;
}

void DapNetworkMonitorWindows::cbRouteChanged(void *, PMIB_IPFORWARD_ROW2 route, MIB_NOTIFICATION_TYPE type) {
    if (!(instance()->m_isMonitoringRunning)) {
        return;
    }
    switch (type) {
    case MibAddInstance:
        if (route->NextHop.Ipv4.sin_addr.S_un.S_addr == 0) {
            emit instance()->sigTunGatewayDefined();
            qWarning() << "Default gateway is set with if metric " << route->Metric;
            /*if (TunTap::getInstance()) {
                TunTap::getInstance().enableDefaultRoutes(route->InterfaceIndex, false);
            }*/
        }
        break;
    case MibDeleteInstance:
        if (route->NextHop.Ipv4.sin_addr.S_un.S_addr == 0) {
            emit instance()->sigTunGatewayUndefined();
            qWarning() << "Default gateway is removed from route table";
        }
        break;
    case MibParameterNotification:
        qWarning() << "Some changes touched the route table";
        break;
    default:
        break;
    }
}

void DapNetworkMonitorWindows::cbIfaceChanged(void *, PMIB_IPINTERFACE_ROW row, MIB_NOTIFICATION_TYPE type) {
    if (!(instance()->m_isMonitoringRunning)) {
        return;
    }
    switch (type) {
    case MibAddInstance:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] enabled";
        if (row->InterfaceIndex == TunTap::getInstance().getTunTapAdapterIndex() ||
                row->InterfaceIndex == TunTap::getInstance().getDefaultAdapterIndex()) {
            emit instance()->sigInterfaceDefined();
        } else if (TunTap::getInstance()) {
            TunTap::getInstance().enableDefaultRoutes(row->InterfaceIndex, false);
        }
        break;
    case MibDeleteInstance:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] disabled";
        if (//row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == TunTap::getInstance().getDefaultAdapterIndex()) {
            emit instance()->sigInterfaceUndefined();
        }
        break;
    case MibParameterNotification:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] settings changed";
        MIB_IF_ROW2 row2;
        ZeroMemory(&row2, sizeof(row2));
        row2.InterfaceIndex = row->InterfaceIndex;//;
        GetIfEntry2(&row2);
        if (row2.MediaConnectState == MediaConnectStateConnected) {
            qWarning() << "Adapter [ " << row->InterfaceIndex << " ] enabled";
            emit instance()->sigInterfaceDefined();

            if (TunTap::getInstance() and (row2.InterfaceIndex != TunTap::getInstance().getDefaultAdapterIndex()) and (row2.InterfaceIndex != TunTap::getInstance().getTunTapAdapterIndex())) {
                TunTap::getInstance().enableDefaultRoutes(row->InterfaceIndex, false);
            }
        } else {
            qWarning() << "Adapter [ " << row->InterfaceIndex << " ] disabled";
            if (row2.InterfaceIndex == TunTap::getInstance().getDefaultAdapterIndex()) {
                emit instance()->sigInterfaceUndefined();
            }
        }
        break;
    default:
        break;
    }
}

void DapNetworkMonitorWindows::internalWorker() {
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
