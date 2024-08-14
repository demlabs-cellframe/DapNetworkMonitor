#pragma once
#include "DapNetworkMonitorAbstract.h"
#include "DapMonitorCmdProgram.h"
#include "dap_network_monitor.h"
#include <QString>
#include <atomic>

class DapNetworkMonitorLinux : public DapNetworkMonitorAbstract
{
    Q_OBJECT

private:
    static void cbMonitorNotification(const dap_network_notification_t notification);
    static bool checkTunnelGw();

    DapNetworkMonitorLinux(QObject *parent = Q_NULLPTR);
    DapNetworkMonitorLinux(const DapNetworkMonitorLinux&) = delete;
    DapNetworkMonitorLinux& operator=(const DapNetworkMonitorLinux&) = delete;

    QString m_tunnelGateway;
    std::atomic<bool> m_isMonitoringRunning;

public:
    static DapNetworkMonitorLinux* instance()
    { static DapNetworkMonitorLinux client; return &client; }

    bool isTunDriverInstalled() const override;

    void setTunnelGateway(const QString &gateway) { m_tunnelGateway = gateway; }
    QString tunnelGateway() const { return m_tunnelGateway; }

signals:

public slots:
    bool monitoringStart() override;
    bool monitoringStop() override;
    bool handleNetworkFailure() override;
};
