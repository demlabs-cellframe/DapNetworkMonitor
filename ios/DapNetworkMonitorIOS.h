#ifndef DAPNETWORKMONITORIOS_H
#define DAPNETWORKMONITORIOS_H


#include "../DapNetworkMonitorAbstract.h"

class DapNetworkMonitorIOS : public DapNetworkMonitorAbstract
{
    Q_OBJECT
public:
    explicit DapNetworkMonitorIOS(QObject *parent = Q_NULLPTR);

    static DapNetworkMonitorIOS* instance()
        { static DapNetworkMonitorIOS client; return &client; }

    bool isTunDriverInstalled() const override;
    bool monitoringStart() override;
    bool monitoringStop() override;

signals:

public slots:
    void procErr(const int, const QString&);
};

#endif // DAPNETWORKMONITORIOS_H