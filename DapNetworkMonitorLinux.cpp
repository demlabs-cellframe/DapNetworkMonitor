#include "DapNetworkMonitorLinux.h"
#include <QProcess>

void DapNetworkMonitorLinux::cbMonitorNotification(const dap_network_notification_t notification)
{
    auto instance = DapNetworkMonitorLinux::instance();

    switch(notification.type) {
    case IP_ADDR_ADD:
    case IP_ADDR_REMOVE: {
        qInfo() << QString("Interface %1 %2 has IP address %3")
                       .arg(notification.addr.interface_name)
                       .arg((notification.type == IP_ADDR_ADD ? "now" : "no longer"))
                       .arg(notification.addr.s_ip);

        if(notification.type == IP_ADDR_ADD){
            emit instance->sigInterfaceDefined();
            DapNetworkMonitorLinux::instance()->sltSetAdpDefined(true);
        } else {
            emit instance->sigInterfaceUndefined();
            DapNetworkMonitorLinux::instance()->sltSetAdpDefined(false);
        }
        break;
    }

    case IP_ROUTE_ADD:
    case IP_ROUTE_REMOVE: {
        qDebug() << QString("%1 route to destination --> %2/%3 proto %4 and gateway %5")
                        .arg((notification.type == IP_ROUTE_ADD ? "Add" : "Delete"))
                        .arg(notification.route.s_destination_address)
                        .arg(notification.route.netmask)
                        .arg(notification.route.protocol)
                        .arg(notification.route.s_gateway_address);

        if (notification.type == IP_ROUTE_REMOVE) {
            if(notification.route.destination_address == DAP_ADRESS_UNDEFINED &&
                notification.route.gateway_address != DAP_ADRESS_UNDEFINED) {
                QString gatewayAddr(notification.route.s_gateway_address);
                if(gatewayAddr == instance->m_tunnelGateway) {
                    if (checkTunnelGw()) {
                        qInfo() << "Tunnel gateway is still defined";
                        emit instance->sigTunGatewayDefined();
                    } else {
                        qInfo() << "Tunnel gateway is undefined";
                        emit instance->sigTunGatewayUndefined();
                    }
                } else {
                    qInfo() << "Other gateway is undefined";
                    emit instance->sigOtherGatewayUndefined();
                }
            } else if(notification.route.destination_address != DAP_ADRESS_UNDEFINED &&
                       notification.route.gateway_address != DAP_ADRESS_UNDEFINED) {
                if(instance->isUpstreamRoute(notification.route.s_destination_address,
                                              notification.route.s_gateway_address)) {
                    qInfo() << "Upstream route is undefined";
                    emit instance->sigUpstreamRouteUndefined();
                }
            }
        } else if (notification.type == IP_ROUTE_ADD) {
            if(notification.route.destination_address == DAP_ADRESS_UNDEFINED &&
                notification.route.gateway_address != DAP_ADRESS_UNDEFINED) {
                QString gatewayAddr(notification.route.s_gateway_address);

                if(gatewayAddr == instance->m_tunnelGateway) {
                    qInfo() << "Tunnel gateway is defined";
                    emit instance->sigTunGatewayDefined();
                } else {
                    qInfo() << "Other gateway is defined";
                    emit instance->sigOtherGatewayDefined(gatewayAddr);
                }
            } else if(notification.route.destination_address != DAP_ADRESS_UNDEFINED &&
                       notification.route.gateway_address != DAP_ADRESS_UNDEFINED) {
                if(instance->isUpstreamRoute(notification.route.s_destination_address,
                                              notification.route.s_gateway_address)) {
                    qInfo() << "Upstream route is defined";
                    emit instance->sigUpstreamRouteDefined();
                }
            }
        }
        break;
    }

    case IP_LINK_NEW:
    case IP_LINK_DEL: {
        qInfo() << QString("Interface %1 is %2 %3 %4")
                       .arg(notification.link.interface_name)
                       .arg(( (notification.type == IP_LINK_NEW && notification.link.is_up) ? "UP" : "DOWN") )
                       .arg((notification.type == IP_LINK_NEW ? "and" : ""))
                       .arg((notification.type == IP_LINK_NEW ?
                                 (notification.link.is_running ? "RUNNING" : "NOT RUNNING")
                                                              : ""));

        if(notification.type == IP_LINK_DEL)
            emit instance->sigInterfaceUndefined();
        else
            emit instance->sigInterfaceDefined();
        break;
    }

    default:
        qWarning() << "Unknown notification type received";
    }

    emit instance->sigRouteChanged();
}

bool DapNetworkMonitorLinux::checkTunnelGw()
{
    auto instance = DapNetworkMonitorLinux::instance();

    if (instance->m_tunnelGateway.isEmpty()) {
        qWarning() << "Tunnel gateway is not set.";
        return false;
    }

    QProcess process;
    QString command = QString("ip route show | grep 'UG' | grep %1").arg(instance->m_tunnelGateway);
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        return true;
    } else {
        qWarning() << "Failed to check tunnel gateway:" << process.readAllStandardError();
        return false;
    }
}

bool DapNetworkMonitorLinux::handleNetworkFailure() {
    qDebug() << "Attempting to recover network connectivity...";

    QProcess process;
    QStringList interfaces = {"eth0", "enp0s3", "wlp2s0"};

    for (const QString &iface : interfaces) {
        QString command = QString("sudo ifconfig %1 down && sudo ifconfig %1 up").arg(iface);
        qDebug() << "Executing command:" << command;
        process.start("bash", QStringList() << "-c" << command);
        process.waitForFinished();

        if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            qDebug() << QString("Network interface %1 restarted successfully.").arg(iface);
            emit sigInterfaceDefined();
            return true;
        } else {
            qWarning() << QString("Failed to restart network interface %1:").arg(iface) << process.readAllStandardError();
        }
    }

    qWarning() << "All attempts to recover network connectivity failed.";
    return false;
}

DapNetworkMonitorLinux::DapNetworkMonitorLinux(QObject *parent):
    DapNetworkMonitorAbstract(parent)
{
    m_isTunGatewayDefined.store(true);
    m_isInterfaceDefined.store(true);
    m_isHostReachable.store(true);
    m_isOtherGatewayDefined.store(true);
}

bool DapNetworkMonitorLinux::isTunDriverInstalled() const
{
    return true;
}

bool DapNetworkMonitorLinux::monitoringStart()
{
    qDebug() << "Start network monitoring";
    if(m_isMonitoringRunning) {
        qWarning() << "Network monitoring already running";
        return true;
    }

    if(dap_network_monitor_init(cbMonitorNotification) == 0) {
        m_isMonitoringRunning = true;
        qDebug() << "Network monitoring started successfully.";
    } else {
        qWarning() << "Failed to start network monitoring.";
        m_isMonitoringRunning = false;
    }

    return m_isMonitoringRunning;
}

bool DapNetworkMonitorLinux::monitoringStop()
{
    qDebug() << "Stop network monitoring";
    if (!m_isMonitoringRunning) {
        qWarning() << "Network monitoring is not running.";
        return false;
    }

    dap_network_monitor_deinit();
    m_isMonitoringRunning = false;
    qDebug() << "Network monitoring stopped.";
    return true;
}
