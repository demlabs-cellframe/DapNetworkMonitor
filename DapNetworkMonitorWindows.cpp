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
            qWarning() << "Default gateway is set";
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
        if (row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == instance()->m_DefaultAdapterIndex) {
            emit instance()->sigInterfaceDefined();
        }
        break;
    case MibDeleteInstance:
        qWarning() << "Adapter [ " << row->InterfaceIndex << " ] disabled";
        if (row->InterfaceIndex == instance()->m_TapAdapterIndex ||
                row->InterfaceIndex == instance()->m_DefaultAdapterIndex) {
            emit instance()->sigInterfaceUndefined();
        }
        break;
    case MibParameterNotification:
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
