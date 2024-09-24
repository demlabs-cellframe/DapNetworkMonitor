#include "DapNetworkMonitorDarwin.h"
#include <QFileInfo>
#include <stdio.h>
#include <string.h>
#include <QProcess>


static void callbackForChangedInterfaces (const void* a_key, const void* a_value, void* a_context)
{
    qInfo() << "Key - " << a_key << " value - " << a_value; // There is no usefull information from this so far
}


void DapNetworkMonitorDarwin::cbMonitorNotification(SCDynamicStoreRef a_store, CFArrayRef a_changedKeys, void *a_info)
{
    // Count of available names in ArrayRef
    CFIndex nameCount = CFArrayGetCount( a_changedKeys );

    //Iterate through the CFArrayRef and fill the vector
    for( int i = 0; i < nameCount ; ++i  )
    {
        CFStringRef l_sName = (CFStringRef)CFArrayGetValueAtIndex( a_changedKeys, i );
        const char *l_cstrName = CFStringGetCStringPtr( l_sName , kCFStringEncodingMacRoman );
        //qInfo() << "Interface " << l_cstrName << " is changed";

        CFDictionaryRef __values = (CFDictionaryRef)SCDynamicStoreCopyValue(a_store, l_sName);
        if (__values == NULL)
        {
            qWarning() << "Interface " << l_cstrName << " is undefined";
            emit instance()->interfaceUndefined();
            return;
        }
        else
        {
            qInfo() << "Defined new or changed interface " << l_cstrName;
        }

        CFDictionaryApplyFunction(__values, callbackForChangedInterfaces, NULL);
    }

    if (instance()->isTunGatewayDefinedInnerCheck())
        emit instance()->sigTunGatewayDefined();
    else if (instance()->isOtherGatewayDefinedInnerCheck())
        //emit instance()->sigOtherGatewayDefined(); // TODO More deep check, because default gateway may be false route from wifi
        emit instance()->sigOtherGatewayUndefined();
    else
    {
        emit instance()->sigTunGatewayUndefined();
        instance()->handleNetworkFailure();
    }
}

bool DapNetworkMonitorDarwin::handleNetworkFailure()
{
    qDebug() << "Attempting to recover network connectivity...";

    QProcess process;
    QString command = "sudo ifconfig en0 down && sudo ifconfig en0 up";
    process.start(command);
    process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        qDebug() << "Network interface en0 restarted successfully.";
    }
    else
    {
        qWarning() << "Failed to restart network interface en0:" << process.readAllStandardError();
    }

    emit sigInterfaceDefined();

    return true;
}

DapNetworkMonitorDarwin::DapNetworkMonitorDarwin(QObject *parent):
    DapNetworkMonitorAbstract(parent)
{ 
    m_isTunGatewayDefined.store(true);
    m_isInterfaceDefined.store(true);
    m_isHostReachable.store(true);
    m_isOtherGatewayDefined.store(true);
}

bool DapNetworkMonitorDarwin::isTunDriverInstalled() const
{
    return true; // TODO make com.apple.developer.networking.networkextension entitlement check
}


bool DapNetworkMonitorDarwin::monitoringStart()
{
    qDebug() << "Start network monitoring";
    if(m_isMonitoringRunning == true)
    {
        qWarning() << "Network monitoring already running";
    }

    if(dap_network_monitor_init(cbMonitorNotification) == 0)
    {
        m_isMonitoringRunning = true;
    }

    return m_isMonitoringRunning;
}
bool DapNetworkMonitorDarwin::monitoringStop()
{
    qDebug() << "Stop network monitoring";
    dap_network_monitor_deinit();

    m_isMonitoringRunning = false;
    return true;
}

/**
 * @brief SapNetworkMonitorDarwin::isOtherGatewayDefinedInnerCheck
 * @return
 */
bool DapNetworkMonitorDarwin::isOtherGatewayDefinedInnerCheck() const
{
    int ret;
    if(m_tunnelGateway.size()>0)
    {
        ret =  ::system(QString("netstat -nr |grep default | grep --invert-match '%1' > /dev/null").arg(m_tunnelGateway)
                        .toLatin1().constData());
        if( ret!= 0)
        {
            qDebug() << "[-] No default gateway, checking direct route to upstream";
            ret =  ::system(QString("netstat -nr |grep '%1' | grep '%2' > /dev/null")
                            .arg(m_serverAddress).arg(m_defaultGateway)
                            .toLatin1().constData());
            if(ret==0)
            {
                qDebug() << "[+] Direct route to "<< m_serverAddress <<" is present, checking if its actual";
                ret =  ::system(QString("netstat -nr | awk '{print $1;}' |egrep '%1'$ > /dev/null").arg(m_defaultGateway)
                                .toLatin1().constData());
                if(ret==0)
                {
                    qDebug() << "[+] Route to "<< m_defaultGateway <<" is present";
                }
                else
                {
                    qDebug() << "[-] No route to "<< m_defaultGateway <<" in the routing table";
                }
            }
            else
                qDebug() << "[-] No direct route to "<< m_serverAddress <<" in the routing table";
        }

    }
    else
        ret =  ::system(QString("netstat -nr |grep default > /dev/null").toLatin1().constData());

    if(ret == 0 )
    {
        qInfo() << "[+] Others gateway defined";
        return true;
    }
    else
    {
        qInfo() << "[-] Others gateway undefined";
        return false;
    }
}

/**
 * @brief SapNetworkMonitorDarwin::isTunGatewayDefinedInnerCheck
 * @return
 */
bool DapNetworkMonitorDarwin::isTunGatewayDefinedInnerCheck() const
{
    int ret;
    if(m_tunnelGateway.size()>0)
        ret =  ::system(QString("netstat -nr |grep default | grep %1 > /dev/null ").arg(m_tunnelGateway)
                        .toLatin1().constData());
    else
        ret =  ::system(QString("netstat -nr |grep default > /dev/null").toLatin1().constData());

    if(ret == 0 )
    {
        qInfo() << "[+] Tunnel gateway defined";
        return true;
    }
    else
    {
        qInfo() << "[-] Tunnel gateway undefined";
        return false;
    }
}
