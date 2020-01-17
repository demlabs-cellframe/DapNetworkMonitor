#pragma once

#include "DapNetworkMonitorAbstract.h"

class DapNetworkMonitorIOS : public DapNetworkMonitorAbstract
{
    Q_OBJECT
public:
    explicit DapNetworkMonitorIOS(QObject *parent = Q_NULLPTR);

    static DapNetworkMonitorIOS *instance() {
        static DapNetworkMonitorIOS me;
        return &me;
    }

    bool isTunDriverInstalled()     const override;
    bool monitoringStart()          override;
    bool monitoringStop()           override;
signals:

public slots:
};
