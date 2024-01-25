#pragma once
#import "DapNetworkMonitorAbstract.h"

class DapNetworkMonitorIOS : public DapNetworkMonitorAbstract
{
    Q_OBJECT
public:
    explicit DapNetworkMonitorIOS(QObject *parent = Q_NULLPTR);

    static DapNetworkMonitorIOS* instance()
    {  }

//     bool isTunDriverInstalled() const override;
//     bool monitoringStart();
//     bool monitoringStop() override;

// signals:

// public slots:
//     void procErr(const int, const QString&);
};
