#include <winsock2.h>
#include <iphlpapi.h>

#include <QThread>

#include "DapNetworkMonitorWindows.h"
#include "tuntap.h"

DapNetworkMonitorWindows::DapNetworkMonitorWindows(QObject *parent):
    DapNetworkMonitorAbstract(parent)
{
    qInfo() << "starting Windows network monitor";
     QtConcurrent::run(this, &DapNetworkMonitorWindows::internalWorker);
}

char *DapNetworkMonitorWindows::readRegKey(HKEY hKey, LPCSTR regSubKey, LPCSTR val) {
    size_t bufSize = 128;
    char *ret = (char*)malloc(bufSize);
    DWORD dwSize = (DWORD)bufSize;
    LSTATUS err = RegGetValueA(hKey, regSubKey, val, RRF_RT_REG_SZ, NULL, (void*)ret, &dwSize);
    if (err == ERROR_SUCCESS) {
        return ret;
    } else {
        free(ret);
        return NULL;
    }
}

bool DapNetworkMonitorWindows::isTunDriverInstalled() const
{
    const char keyPath[] = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
    HKEY baseKey;
    LSTATUS err = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0
                  ,KEY_ENUMERATE_SUB_KEYS | KEY_WOW64_64KEY | KEY_READ
                  ,&baseKey);
    if (err != ERROR_SUCCESS) {
        return NULL;
    }
    DWORD index;
    char tmp[128] = {'\0'};
    for (index = 0; ; ++index) {
        char hKey[MAX_PATH];
        DWORD len = MAX_PATH;
        if (RegEnumKeyExA(baseKey, index, hKey, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
            break;
        }
        char *tmp2 = readRegKey(baseKey, hKey, "ComponentId");
        if (tmp2) {
            strcpy(tmp, tmp2);
            free(tmp2);
            if (strcmp(tmp, "tap0901") == 0) return true;
            memset(tmp, '\0', sizeof(tmp));
        }
    }
    return false;
}

bool DapNetworkMonitorWindows::isTunGatewayDefined() const
{
    return m_isTunGatewayDefined;
}
bool DapNetworkMonitorWindows::isOtherGatewayDefined() const
{
    return m_isOtherGatewayDefined;
}

bool DapNetworkMonitorWindows::monitoringStart()
{
    QMutexLocker lock(&mutex);
    m_isMonitoringRunning = true;
    return m_isMonitoringRunning;
}

bool DapNetworkMonitorWindows::monitoringStop()
{
    QMutexLocker lock(&mutex);
    m_isMonitoringRunning = false;
    return m_isMonitoringRunning;
}

void DapNetworkMonitorWindows::internalWorker() {
    HANDLE hAddrChange, hRouteChange;
    OVERLAPPED ovAddrChange, ovRouteChange;

    ovAddrChange.hEvent  = WSACreateEvent();
    ovRouteChange.hEvent = WSACreateEvent();

    bool bNoDefaultGateWay = false;

    while (true)
    {
        DWORD dwRet = NotifyAddrChange(&hAddrChange, &ovAddrChange);
        if ( dwRet != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING ) {
            qDebug() << "NotifyAddrChange failed, error code ==" << QString::number(WSAGetLastError())
                     << ", shutdown DapNetworkWindowsMonitor";
            return;
        }

        if ( (dwRet = NotifyRouteChange(&hRouteChange, &ovRouteChange)) != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING ) {
            qDebug() << "NotifyRouteChange failed, error code ==" << QString::number(WSAGetLastError())
                     << ", shutdown DapNetworkWindowsMonitor";
            return;
        }

        HANDLE hEvents[2] = {ovAddrChange.hEvent, ovRouteChange.hEvent};

        dwRet = WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE), hEvents, false, INFINITE);

        if ( m_isMonitoringRunning && ( dwRet == WAIT_OBJECT_0 || dwRet == (WAIT_OBJECT_0 + 1) ) ) {
            qDebug() << "[DapNetworkWindowsMonitor] : some event happend";
            emit sigRouteChanged();

            if (!TunTap::getInstance().getDefaultGateWayCount() && !bNoDefaultGateWay) {
                bNoDefaultGateWay = true;
                qDebug() << "[SapNetworkWindowsMonitor] : happend sigDefaultGatewayUndefined situation";
                emit sigTunGatewayUndefined();
            }

            if (TunTap::getInstance().getDefaultGateWayCount() && bNoDefaultGateWay) {
                bNoDefaultGateWay = false;
                qDebug() << "[SapNetworkWindowsMonitor] : happend sigDefaultGatewayDefined situation";
                emit sigTunGatewayDefined();
            }
        }
    }
}
