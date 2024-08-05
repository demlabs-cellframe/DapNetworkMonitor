#ifndef NETWORKMONITORABSTRACT_H
#define NETWORKMONITORABSTRACT_H

#include <QDebug>
#include <QObject>
#include <atomic>

class DapNetworkMonitorAbstract : public QObject
{
    Q_OBJECT
protected:
    // default gateway before install tunneling
    QString m_defaultGateway;
    QString m_tunnelDestination;
    QString m_serverAddress;
    QString m_tunnelGateway;

    std::atomic<bool> m_isMonitoringRunning, m_isTunGatewayDefined, m_isOtherGatewayDefined, m_isInterfaceDefined, m_isHostReachable;
public:
    explicit DapNetworkMonitorAbstract(QObject *parent = Q_NULLPTR);

    virtual ~DapNetworkMonitorAbstract() {}

    virtual bool isTunDriverInstalled() const = 0;

    virtual bool isTunGatewayDefined() final   { return  m_isTunGatewayDefined.load(); }
    virtual bool isOtherGatewayDefined() final { return  m_isOtherGatewayDefined.load(); }
    virtual bool isInterfaceDefined() final    { return m_isInterfaceDefined.load(); }
    virtual bool isMonitoringRunning() const final { return m_isMonitoringRunning.load(); }
    virtual bool isHostReachable() const final { return m_isHostReachable.load(); }

    bool isUpstreamRoute(const QString& destination, const QString& gateway)  {
        return destination == m_serverAddress && gateway == m_defaultGateway;
    }

signals:
    void sigRouteChanged();
    void sigOtherGatewayDefined(const QString& gateway);
    void sigOtherGatewayUndefined();
    void sigTunGatewayDefined();
    void sigTunGatewayUndefined();
    void sigUpstreamRouteDefined();
    void sigUpstreamRouteUndefined();
    void sigUpstreamRouteGatewayUndefined();

    void sigInterfaceUndefined();
    void sigInterfaceDefined();

    void sigMonitoringFinished();
    void sigMonitoringStarted();
    void sigMonitoringStartError();

    void sigTunCreate();

public slots:
    void sltSetDefaultGateway(const QString& gw) { m_defaultGateway = gw; }
    void sltSetTunGateway(const QString& gw) { m_tunnelGateway = gw; }
    void sltSetServerAddress(const QString& servAddr) { m_serverAddress = servAddr; }
    void sltSetTunnelDestination(const QString& tunDest) { m_tunnelDestination = tunDest; }
    void sltSetHostReachable(bool b) { m_isHostReachable.store(b); }
    void sltSetAdpDefined(bool b) { m_isInterfaceDefined.store(b); }
    virtual bool handleNetworkFailure() = 0;

    // returns true if operation successfully
    virtual bool monitoringStart() = 0;
    virtual bool monitoringStop() = 0;
};


#endif // NETWORKMONITORABSTRACT_H
