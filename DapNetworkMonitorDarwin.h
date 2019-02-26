#ifndef NETWORKMONITORMACOS_H
#define NETWORKMONITORMACOS_H

#include "DapNetworkMonitorAbstract.h"
#include "dap_network_monitor.h"

class DapNetworkMonitorDarwin : public DapNetworkMonitorAbstract
{
    Q_OBJECT
private:

    DapNetworkMonitorDarwin(QObject *parent = Q_NULLPTR);
    DapNetworkMonitorDarwin(const DapNetworkMonitorDarwin&) = delete;
    DapNetworkMonitorDarwin& operator=(const DapNetworkMonitorDarwin&) = delete;

    bool isTunGatewayDefinedInnerCheck() const;
    bool isOtherGatewayDefinedInnerCheck() const;

    static void cbMonitorNotification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info);

public:
    static DapNetworkMonitorDarwin* instance()
        {static DapNetworkMonitorDarwin client; return &client;}

    bool isTunDriverInstalled() const override;

public slots:
    bool monitoringStart() override;
    bool monitoringStop() override;
};

#endif // NETWORKMONITORMACOS_H
