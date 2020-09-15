#ifndef NETWORKMONITORWINDOWS_H
#define NETWORKMONITORWINDOWS_H

#include "DapNetworkMonitorAbstract.h"
#include <QtConcurrent/QtConcurrent>
#include "tuntap.h"

class DapNetworkMonitorWindows : public DapNetworkMonitorAbstract
{
    Q_OBJECT
public:
    explicit DapNetworkMonitorWindows(QObject *parent = Q_NULLPTR);

    static DapNetworkMonitorWindows *instance() {
        static DapNetworkMonitorWindows me;
        return &me;
    }


    bool isTunDriverInstalled() const override;
    bool monitoringStart() override;
    bool monitoringStop() override;

private:
    void internalWorker();
    QMutex mutex;

    ulong m_TapAdapterIndex = 0, m_DefaultAdapterIndex = 0;
    static void cbRouteChanged(void *ctx, PMIB_IPFORWARD_ROW2 route, MIB_NOTIFICATION_TYPE type);
    static void cbIfaceChanged(void *ctx, PMIB_IPINTERFACE_ROW row, MIB_NOTIFICATION_TYPE type);
signals:

public slots:
    void sltSetTapIfIndex   (const ulong index) { m_TapAdapterIndex = index; }
    void sltSetIfIndex      (const ulong index) { m_DefaultAdapterIndex = index; }
};

#endif // NETWORKMONITORWINDOWS_H
