#ifndef NETWORKMONITORANDROID_H
#define NETWORKMONITORANDROID_H

#include "DapNetworkMonitorAbstract.h"

class DapNetworkMonitorAndroid : public DapNetworkMonitorAbstract
{
    Q_OBJECT
public:
    explicit DapNetworkMonitorAndroid(QObject *parent = Q_NULLPTR);

    static DapNetworkMonitorAndroid* instance()
    { 
        static DapNetworkMonitorAndroid client; 
        return &client; 
    }

    bool isTunDriverInstalled() const override;
    bool monitoringStart() override;
    bool monitoringStop() override;

public slots:
    void procErr(const int, const QString&);
};

#endif // NETWORKMONITORANDROID_H
