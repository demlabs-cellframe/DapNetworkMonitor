#include <iostream>
#include <qdebug.h>
#include "tuntap.h"

static const char *strerror_win32(DWORD errnum)
{
    switch (errnum)
    {
        case ERROR_GEN_FAILURE:
            return "General failure (ERROR_GEN_FAILURE)";
        case ERROR_IO_PENDING:
            return "I/O Operation in progress (ERROR_IO_PENDING)";
        case ERROR_IO_INCOMPLETE:
            return "I/O Operation in progress (WSA_IO_INCOMPLETE)";
        case WSAEINTR:
            return "Interrupted system call (WSAEINTR)";
        case WSAEBADF:
            return "Bad file number (WSAEBADF)";
        case WSAEACCES:
            return "Permission denied (WSAEACCES)";
        case WSAEFAULT:
            return "Bad address (WSAEFAULT)";
        case WSAEINVAL:
            return "Invalid argument (WSAEINVAL)";
        case WSAEMFILE:
            return "Too many open files (WSAEMFILE)";
        case WSAEWOULDBLOCK:
            return "Operation would block (WSAEWOULDBLOCK)";
        case WSAEINPROGRESS:
            return "Operation now in progress (WSAEINPROGRESS)";
        case WSAEALREADY:
            return "Operation already in progress (WSAEALREADY)";
        case WSAEDESTADDRREQ:
            return "Destination address required (WSAEDESTADDRREQ)";
        case WSAEMSGSIZE:
            return "Message too long (WSAEMSGSIZE)";
        case WSAEPROTOTYPE:
            return "Protocol wrong type for socket (WSAEPROTOTYPE)";
        case WSAENOPROTOOPT:
            return "Bad protocol option (WSAENOPROTOOPT)";
        case WSAEPROTONOSUPPORT:
            return "Protocol not supported (WSAEPROTONOSUPPORT)";
        case WSAESOCKTNOSUPPORT:
            return "Socket type not supported (WSAESOCKTNOSUPPORT)";
        case WSAEOPNOTSUPP:
            return "Operation not supported on socket (WSAEOPNOTSUPP)";
        case WSAEPFNOSUPPORT:
            return "Protocol family not supported (WSAEPFNOSUPPORT)";
        case WSAEAFNOSUPPORT:
            return "Address family not supported by protocol family (WSAEAFNOSUPPORT)";
        case WSAEADDRINUSE:
            return "Address already in use (WSAEADDRINUSE)";
        case WSAENETDOWN:
            return "Network is down (WSAENETDOWN)";
        case WSAENETUNREACH:
            return "Network is unreachable (WSAENETUNREACH)";
        case WSAENETRESET:
            return "Net dropped connection or reset (WSAENETRESET)";
        case WSAECONNABORTED:
            return "Software caused connection abort (WSAECONNABORTED)";
        case WSAECONNRESET:
            return "Connection reset by peer (WSAECONNRESET)";
        case WSAENOBUFS:
            return "No buffer space available (WSAENOBUFS)";
        case WSAEISCONN:
            return "Socket is already connected (WSAEISCONN)";
        case WSAENOTCONN:
            return "Socket is not connected (WSAENOTCONN)";
        case WSAETIMEDOUT:
            return "Connection timed out (WSAETIMEDOUT)";
        case WSAECONNREFUSED:
            return "Connection refused (WSAECONNREFUSED)";
        case WSAELOOP:
            return "Too many levels of symbolic links (WSAELOOP)";
        case WSAENAMETOOLONG:
            return "File name too long (WSAENAMETOOLONG)";
        case WSAEHOSTDOWN:
            return "Host is down (WSAEHOSTDOWN)";
        case WSAEHOSTUNREACH:
            return "No Route to Host (WSAEHOSTUNREACH)";
        case WSAENOTEMPTY:
            return "Directory not empty (WSAENOTEMPTY)";
        case WSAEPROCLIM:
            return "Too many processes (WSAEPROCLIM)";
        case WSAEUSERS:
            return "Too many users (WSAEUSERS)";
        case WSAEDQUOT:
            return "Disc Quota Exceeded (WSAEDQUOT)";
        case WSAESTALE:
            return "Stale NFS file handle (WSAESTALE)";
        case WSASYSNOTREADY:
            return "Network SubSystem is unavailable (WSASYSNOTREADY)";
        case WSAVERNOTSUPPORTED:
            return "WINSOCK DLL Version out of range (WSAVERNOTSUPPORTED)";
        case WSANOTINITIALISED:
            return "Successful WSASTARTUP not yet performed (WSANOTINITIALISED)";
        case WSAEREMOTE:
            return "Too many levels of remote in path (WSAEREMOTE)";
        case WSAHOST_NOT_FOUND:
            return "Host not found (WSAHOST_NOT_FOUND)";
        default:
            return "Unknown error";
    }
}

TunTap::TunTap()
{
    _ifIndex[0] = _ifIndex[1] = -1;
    hDnsApi     = LoadLibrary(L"dnsapi.dll");
    hNetShell   = LoadLibrary(L"netshell.dll");
    hDhcpDll    = LoadLibrary(L"dhcpcsvc.dll");

    if (hDnsApi)
        DnsFlushResolverCache   = reinterpret_cast<pDnsFlushResolverCache>  (GetProcAddress(hDnsApi, "DnsFlushResolverCache"));
    if (hNetShell)
        NcFreeNetconProperties  = reinterpret_cast<pNcFreeNetconProperties> (GetProcAddress(hNetShell, "NcFreeNetconProperties"));
    if (hDhcpDll)
        pDhcpNotifyProc         = reinterpret_cast<pDHCPNotifyProc>         (GetProcAddress(hDhcpDll, "DhcpNotifyConfigChange"));
}

/**
 * @brief TunTap::ctl_code
 * @param deviceType
 * @param function
 * @param method
 * @param acess
 */
unsigned TunTap::ctl_code(unsigned deviceType, unsigned function, unsigned method, unsigned acess)
{
    return ((deviceType << 16) | (acess << 14) | (function << 2) | method);
}

/**
 * @brief TunTap::tap_control_code
 * @param request
 * @param method
 */
unsigned TunTap::tap_control_code(unsigned request, unsigned method)
{
    return ctl_code(FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS);
}

int TunTap::_getTunAdapterIndex() {
    PIP_INTERFACE_INFO pInfo = reinterpret_cast<PIP_INTERFACE_INFO>(alloca(APIBUFLEN));
    unsigned long ulRetVal = 0, ulOutBufLen = APIBUFLEN;
    int tunNum = -1;
    if ((ulRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen)) == NO_ERROR) {
        for (int i = 0; i < pInfo->NumAdapters; ++i) {
            if (wcswcs(pInfo->Adapter[i].Name, getTapGUID()) != nullptr) {
                tunNum = static_cast<int>(pInfo->Adapter[i].Index);
                qInfo () << "[TunTap] Tun adapter number: " << tunNum;
                goto RET;
            }
        }
    } else {
        qCritical() << "[TunTap] Unable to get interfaces info";
    }
    qCritical() << "[TunTap] Tun adapter number NOT FOUND";
RET:
    return tunNum;
}

/**
 * @brief TunTap::overlappedIoInit
 * @param o
 * @param eventState
 */
void TunTap::overlappedIoInit(overlapped_io *o, bool eventState)
{
   memset(o, 0, sizeof(overlapped_io));
   o->overlapped.hEvent = CreateEvent(nullptr, true/*manual reset*/, eventState, nullptr);
   if (o->overlapped.hEvent == nullptr) {
       qCritical() << "Error: overlappedIoInit: CreateEvent failed";
   }
}

/**
 * @brief TunTap::getInstance
 * @return
 */
TunTap &TunTap::getInstance()
{
    static TunTap _this; return _this;
}

/**
 * @brief TunTap::makeTunTapDevice
 * @return a kernel descriptor of tuntap device
 */
int TunTap::makeTunTapDevice(QString &outTunName) {
    qInfo() << "[TunTap] Creating tuntap device";
    if (enableTapAdapter())
        qInfo() << "[TunTap] TAP adapter connected";
    static WinSecurityParam secAttr;
    _tunWinId = QString::fromWCharArray(getTapGUID());
    QString userModeDevice = "\\\\.\\Global\\";
    userModeDevice += _tunWinId;
    userModeDevice += ".tap";
    _fileHandle = CreateFileA (
                qPrintable(userModeDevice),
                MAXIMUM_ALLOWED,
                FILE_SHARE_READ|FILE_SHARE_WRITE,
                nullptr/*secAttr*/,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_SYSTEM|FILE_FLAG_OVERLAPPED,
                nullptr);

    if (_fileHandle == INVALID_HANDLE_VALUE) {
        QString msg = "Unable to open device file \"";
                msg += userModeDevice;
                msg += '"';
        //throw TunTapException(msg.toStdWString().c_str());
    }

    if (secAttr && secAttr.injectPermissions(_fileHandle))
        qInfo() << "Tap driver switch to maximum allow rules";

    int pStatus {1};
    unsigned long rLen;
    if (!DeviceIoControl ( _fileHandle,
                           tap_control_code(6, METHOD_BUFFERED),
                           &pStatus, sizeof(int),
                           &pStatus, sizeof(int),
                           &rLen, nullptr)) {
        QString msg = "Can not send commad to device \"";
                msg += userModeDevice;
                msg += '"';
        //throw TunTapException(msg.toStdWString().c_str());
    }
    tunDevice = new TunDevice();
    tunDevice->_fileHandle = _fileHandle;
    overlappedIoInit(&tunDevice->reads, false);
    overlappedIoInit(&tunDevice->writes, false);
    tunDevice->rw_handle.read = tunDevice->reads.overlapped.hEvent;
    tunDevice->rw_handle.write = tunDevice->writes.overlapped.hEvent;
    outTunName = _tunWinId + ".tap";

    unsigned long mtu;
    if (DeviceIoControl(_fileHandle, tap_control_code(3, METHOD_BUFFERED),
                        &mtu, sizeof(mtu),
                        &mtu, sizeof(mtu), &rLen, nullptr))
    {
        qInfo () << "TAP-Windows MTU=" << mtu;
    }
    _active = true;
    _ifIndex[1] = _getTunAdapterIndex();
    hInactive = CreateEvent(nullptr, true, false, nullptr);
    return PtrToInt(tunDevice->_fileHandle);
}

bool TunTap::unassignTunAdp() {
    bool status = true;
    PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(alloca(APIBUFLEN));
    u_long ulOutBufLen = APIBUFLEN;
    u_long ulRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    if (ulRetVal == ERROR_SUCCESS) {
        for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
            if (pAdapter->Index == _ifIndex[1]) {
                for (PIP_ADDR_STRING addrStr = &(pAdapter->IpAddressList); addrStr; addrStr = addrStr->Next) {
                    qInfo() << "[TunTap] found unassigned IP on tunnel adapter [" << pAdapter->Index << "], delete it.";
                    u_long err = DeleteIPAddress(addrStr->Context);
                    if (err != NO_ERROR) {
                        qInfo() << "[TunTap] couldn't delete that IP, error " << err;
                        status = false;
                    } else {
                        qInfo() << "[TunTap] old IP address deleted.";
                    }
                }
            }
        }
    } else {
        qCritical() << "[TunTap] failed to obtain adapters info, error " << ulRetVal;
        status = false;
    }
    return status;
}

bool TunTap::dhcpEnabled(ulong ifIndex) {
    PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(alloca(APIBUFLEN));
    u_long ulOutBufLen = APIBUFLEN;
    u_long ulRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    if (ulRetVal == ERROR_SUCCESS) {
        for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
            if ((pAdapter->Index == ifIndex) && ifMap.contains(pAdapter->Index)) {
                ifMap[pAdapter->Index].dhcp = pAdapter->DhcpEnabled;
                qInfo() << "[TunTap] Adapter " << ifIndex << ": dhcp " << pAdapter->DhcpEnabled;
                return pAdapter->DhcpEnabled;
            }
        }
    } else {
        qCritical() << "[TunTap] failed to obtain adapters info, error " << ulRetVal;
        return false;
    }
}

/**
 * @brief TunTap::setAdress
 * @param ip
 * @param gw
 * @return
 */
int TunTap::setAdress(const QString &ip, const QString &gw, const QString &mask) {
    DWORD err;
    u_long rLen;

    qInfo() << "[TunTap] Try to assign params to tun adapter " << _ifIndex[1];
    NTEContext = 0;
    ULONG NTEInstance = 0;
    if ((err = AddIPAddress(inet_addr(qPrintable(ip)),
                                inet_addr(qPrintable(mask)),
                                _ifIndex[1],
                                &NTEContext, &NTEInstance)) == NO_ERROR) {
        qInfo() << "[TunTap] IP address [" << ip << "] assigned to Tun adapter";
    } else if (err == ERROR_OBJECT_ALREADY_EXISTS) {
        qInfo() << "[TunTap] IP address [" << ip << "] is already assigned to Tun adapter";
    } else {
        qCritical() << "[TunTap] IP address couldn't be assigned to Tun adapter, err: " << err;
        return 1;
    }

    QString _gw(gw);
    _gw.chop(1);
    _gw.append('0');
    int pTun[3] = { (int)inet_addr(qPrintable(ip)),
                    (int)inet_addr(qPrintable(_gw)),
                    (int)inet_addr(qPrintable(mask)) };
    if (!DeviceIoControl ( _fileHandle,
                           tap_control_code(10, METHOD_BUFFERED),
                           pTun, 3*sizeof(int),
                           pTun, 3*sizeof(int),
                           &rLen, nullptr)) {
        qCritical() << "[TunTap] Couldn't set Tun adapter network params!";
        return -1;
    } else {
        qInfo() << "[TunTap] Tun adapter got network params: GW ["<< _gw
                 << "], IP [" << ip << "], mask [" << mask << ']';
    }
    return 0;
}

/**
 * @brief TunTap::getDefaultGateWay
 * @return default gateway adress.
 */
QString TunTap::getDefaultGateWay() {
    unsigned long tableSize = APIBUFLEN;
    PMIB_IPFORWARDTABLE pIpRouteTable = reinterpret_cast<PMIB_IPFORWARDTABLE>(alloca(APIBUFLEN));
    QString ret;
    ulong minMetric = 9999;
    DWORD err;
    if ((err = GetIpForwardTable(pIpRouteTable, &tableSize, 0)) == NO_ERROR) {
        for (unsigned long i = 0; i < pIpRouteTable->dwNumEntries; ++i) {
            if ((pIpRouteTable->table[i].dwForwardDest == 0) and (pIpRouteTable->table[i].dwForwardIfIndex != _ifIndex[1])) {
                ifMap[pIpRouteTable->table[i].dwForwardIfIndex].ifRow = pIpRouteTable->table[i];
                _ifIndex[0] = static_cast<int>(pIpRouteTable->table[i].dwForwardIfIndex);
                if (pIpRouteTable->table[i].dwForwardMetric1 < minMetric) {
                    minMetric = pIpRouteTable->table[i].dwForwardMetric1;
                    IN_ADDR ipAddr = {
                        {
                            .S_addr = pIpRouteTable->table[i].dwForwardNextHop
                        }
                    };
                    ret = QString(inet_ntoa(ipAddr));
                }
            }
        }
    } else {
        qCritical() << "[TunTap] Failed to obtain route table, err code: " << err;
    }
    qInfo() << "[TunTap] Default adapter index: " << _ifIndex[0];
    return ret;
}

bool TunTap::deleteCustomRoutes() {
    bool res = true;
    DWORD err;    
    for (auto r : customRoutes) {
        err = DeleteIpForwardEntry(&r);
        IN_ADDR ipAddr = {
            {
                .S_addr = r.dwForwardDest
            }
        };
        if (err == NO_ERROR) {
            qInfo() << "[TunTap] Deleted route [ " << inet_ntoa(ipAddr) << " ]";
        } else {
            res = false;
            qCritical() << "[TunTap] Couldn't delete route [ " << inet_ntoa(ipAddr) << " ], err " << err;
        }
    }
    customRoutes.clear();
    return res;
}

size_t TunTap::getDefaultGateWayCount()
{
    unsigned long tableSize = APIBUFLEN;
    PMIB_IPFORWARDTABLE pIpRouteTable = reinterpret_cast<PMIB_IPFORWARDTABLE>(alloca(APIBUFLEN));
    size_t ret = 0;
    DWORD err;
    if ((err = GetIpForwardTable(pIpRouteTable, &tableSize, 0)) == NO_ERROR) {
        for (unsigned long i = 0; i < pIpRouteTable->dwNumEntries; ++i) {
            if (pIpRouteTable->table[i].dwForwardDest == 0) {
                ++ret;
            }
        }
    } else {
        qCritical() << "[TunTap] Failed to obtain route table, err code: " << err;
    }
    return ret;
}

void TunTap::deleteRoutesByIfIndex(DWORD ifInd) {
    unsigned long tableSize = APIBUFLEN;
    PMIB_IPFORWARDTABLE pIpRouteTable = reinterpret_cast<PMIB_IPFORWARDTABLE>(alloca(APIBUFLEN));
    DWORD err;
    if ((err = GetIpForwardTable(pIpRouteTable, &tableSize, 0)) != NO_ERROR) {
        qCritical() << "[TunTap] Couldn't get route table!";
        return;
    }
    for (int i = 0; i < pIpRouteTable->dwNumEntries; ++i) {
        if (pIpRouteTable->table[i].dwForwardIfIndex == ifInd) {
            IN_ADDR ipAddr = {
                {
                    .S_addr = pIpRouteTable->table[i].dwForwardDest
                }
            };
            err = DeleteIpForwardEntry(&pIpRouteTable->table[i]);
            if (err == NO_ERROR) {
                qInfo() << "[TunTap] Deleted route [ " << inet_ntoa(ipAddr) << " ]";
            } else {
                qCritical() << "[TunTap] Couldn't delete route [ " << inet_ntoa(ipAddr) << " ], err " << err;
            }
        }
    }
}

bool TunTap::defaultRouteDelete() {
    qDebug() << "Deleting default route";
    if (DeleteIpForwardEntry(&ifMap[_ifIndex[0]].ifRow) != ERROR_SUCCESS) {
        qCritical() << "Could not delete default gateway";
        // no make throw here
        return false;
    }
    return true;
}


void TunTap::deleteRoutesByGw(const char* gw) {
    unsigned long tableSize = APIBUFLEN;
    PMIB_IPFORWARDTABLE pIpRouteTable = reinterpret_cast<PMIB_IPFORWARDTABLE>(alloca(APIBUFLEN));
    DWORD err;
    if ((err = GetIpForwardTable(pIpRouteTable, &tableSize, 0)) == NO_ERROR) {
        for (int i = 0; i < pIpRouteTable->dwNumEntries; ++i) {
            IN_ADDR ipAddr = {
                {
                    .S_addr = pIpRouteTable->table[i].dwForwardDest
                }
            };
            if (inet_addr(gw) == pIpRouteTable->table[i].dwForwardDest) {
                err = DeleteIpForwardEntry(&pIpRouteTable->table[i]);
                if (err == NO_ERROR) {
                    qInfo() << "[TunTap] Deleted route to gw [ " << inet_ntoa(ipAddr) << " ]";
                } else {
                    qCritical() << "[TunTap] Couldn't delete route to gw [ " << inet_ntoa(ipAddr) << " ], err " << err;
                }
            }
        }
    }
}

void TunTap::makeRoute(AdapterType adapter, const QString &dest, const QString &gate, ulong metric, const QString &mask, bool addToTable) {
    unsigned long status;
    MIB_IPFORWARDROW pIpWay;
    pIpWay.dwForwardDest    = inet_addr(qPrintable(dest));
    pIpWay.dwForwardMask    = inet_addr(qPrintable(mask));
    pIpWay.dwForwardNextHop = inet_addr(qPrintable(gate));
    pIpWay.dwForwardPolicy  = 0;
    pIpWay.dwForwardMetric1 = metric;
    pIpWay.dwForwardMetric2 = 0;
    pIpWay.dwForwardMetric3 = 0;
    pIpWay.dwForwardMetric4 = 0;
    pIpWay.dwForwardMetric5 = 0;
    pIpWay.dwForwardNextHopAS = 0;
    pIpWay.dwForwardProto   = MIB_IPPROTO_NETMGMT;
    if (adapter == TUN) {
        pIpWay.dwForwardIfIndex = _ifIndex[1];
        pIpWay.dwForwardType    = MIB_IPROUTE_TYPE_INDIRECT;
        pIpWay.dwForwardAge     = 0;
    } else {
        pIpWay.dwForwardIfIndex = _ifIndex[0];
        pIpWay.dwForwardType    = MIB_IPROUTE_TYPE_DIRECT;
        pIpWay.dwForwardAge     = INFINITE;
    }

    if ((status = CreateIpForwardEntry(&pIpWay)) == NO_ERROR) {
        qInfo() << "[TunTap] Created new route from [" << dest << "] to ["
                 << gate << "] with mask [" << mask << ']';
         if (addToTable && (adapter == ETH))
            customRoutes.push_back(pIpWay);
    } else if (status == ERROR_OBJECT_ALREADY_EXISTS) {
        qWarning() << "Route is already present.";
    } else {
        qCritical() << "[TunTap] Couldn't create route from [" << dest << "] to ["
             << gate << "] with mask [" << mask << "], err: " << status;
    }
    return;
}

bool TunTap::determineValidArgs(ulong& metric_eth, ulong& metric_tun) {
    int status;
    PMIB_IPINTERFACE_TABLE ipIfTable = nullptr;
    status = GetIpInterfaceTable(AF_INET, &ipIfTable);
    if (status != ERROR_SUCCESS) {
        qCritical() << "[TunTap] Can't obtain network interfaces data: " << status;
        return false;
    }
    for (uint i = 0; i < ipIfTable->NumEntries; ++i) {
        if (ipIfTable->Table[i].InterfaceIndex == _ifIndex[0]) {
            metric_eth = ipIfTable->Table[i].Metric + 1;
            qInfo() << "[TunTap]: Min default interface metric is: " << metric_eth;
        } else if (ipIfTable->Table[i].InterfaceIndex == _ifIndex[1]) {
            metric_tun = ipIfTable->Table[i].Metric + 1;
            qInfo() << "[TunTap]: Min tun interface metric is: " << metric_tun;
            if(ipIfTable->Table[i].DisableDefaultRoutes) {
                qInfo() << "[TunTap]: Split tunneling disabled";
            }
        }
    }
    FreeMibTable(ipIfTable);
    return true;
}

bool TunTap::enableDefaultRoutes(ulong ifIndex, bool flag) {
    int status;
    PMIB_IPINTERFACE_TABLE ipIfTable = nullptr;
    status = GetIpInterfaceTable(AF_INET, &ipIfTable);
    if (status != ERROR_SUCCESS) {
        qCritical() << "[TunTap] Can't obtain network interfaces data: " << status;
        return false;
    }
    for (uint i = 0; i < ipIfTable->NumEntries; ++i) {
        if (ipIfTable->Table[i].InterfaceIndex == ifIndex) {
            MIB_IPINTERFACE_ROW ipRow = ipIfTable->Table[i];
            if (!ipRow.DisableDefaultRoutes or flag) {
                qInfo() << "[TunTap]: Split tunneling on interface [ " << ifIndex << " ]: " << flag;
                ipRow.SitePrefixLength = 0;
                ipRow.DisableDefaultRoutes = !flag;
                //ipRow.ForwardingEnabled = flag;
                ipRow.Metric = 4280;
                ipRow.UseAutomaticMetric = flag;
                if (status = SetIpInterfaceEntry(&ipRow) != ERROR_SUCCESS) {
                    qDebug() << "[TunTap]: couldn't reset split tunneling on adp [ " << ifIndex << " ] : err " << GetLastError();
                } else {
                    ifMap[ifIndex].defaultOn = flag;
                }
            } // else, attempt to disable when it's already off
        }
    }
    FreeMibTable(ipIfTable);
    return true;
}

bool TunTap::enableDefaultRoutes() {
    int status;
    PMIB_IPINTERFACE_TABLE ipIfTable = nullptr;
    status = GetIpInterfaceTable(AF_INET, &ipIfTable);
    if (status != ERROR_SUCCESS) {
        qCritical() << "[TunTap] Can't obtain network interfaces data: " << status;
        return false;
    }
    for (uint i = 0; i < ipIfTable->NumEntries; ++i) {
        if (ifMap.contains(ipIfTable->Table[i].InterfaceIndex)) {
            MIB_IPINTERFACE_ROW ipRow = ipIfTable->Table[i];
            if (ifMap[ipIfTable->Table[i].InterfaceIndex].defaultOn == 0) {
                qInfo() << "[TunTap]: Enable split tunneling on interface [ " << ipIfTable->Table[i].InterfaceIndex << " ]";
                ipRow.SitePrefixLength = 0;
                ipRow.DisableDefaultRoutes = false;
                //ipRow.ForwardingEnabled = true;
                ipRow.UseAutomaticMetric = true;
                if (status = SetIpInterfaceEntry(&ipRow) != ERROR_SUCCESS) {
                    qDebug() << "[TunTap]: couldn't reset split tunneling on adp [ " << ipIfTable->Table[i].InterfaceIndex << " ] : err " << GetLastError();
                } else {
                    ifMap.remove(ipIfTable->Table[i].InterfaceIndex);
                }
            }
        }
    }
    FreeMibTable(ipIfTable);
    return true;
}

/**
 * @brief TunTap::getDefaultAdapterIndex
 * @return
 */
int TunTap::getDefaultAdapterIndex()
{
    return _ifIndex[0];
}

/**
 * @brief TunTap::getTunTapAdapterIndex
 * @return
 */
int TunTap::getTunTapAdapterIndex()
{
    return _ifIndex[1];
}

void TunTap::doCloseTun()
{
    SetEvent(hInactive);
    _active = false;
}

void TunTap::wakeupTun() {
    ResetEvent(hInactive);
    _active = true;
}

void TunTap::closeTun()
{
    u_long err = DeleteIPAddress(NTEContext);
    if (err != NO_ERROR) {
        qCritical() << "[TunTap] Couldn't unassign IP address of tun adapter, err: " << err;
        qWarning() << "Attempt to update net table entry context...";
        unassignTunAdp();
    }
    CloseHandle(hInactive);
    hInactive = nullptr;
    CloseHandle(_fileHandle);
    CloseHandle(tunDevice->writes.overlapped.hEvent);
    CloseHandle(tunDevice->reads.overlapped.hEvent);
    delete tunDevice;
}

TunTap::operator bool()
{
    return _active;
}

/**
 * @brief TunTap::getDefaultDNSList
 * @return
 */
/* QStringList TunTap::getDefaultDNSList()
{
    FIXED_INFO *pFixedInfo;
    unsigned long ulOutBufLen;
    unsigned long ulRetVal;

    QStringList ret;
    ulOutBufLen = sizeof(FIXED_INFO);
    pFixedInfo =  (FIXED_INFO*)::malloc(ulOutBufLen);

    if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        ::free(pFixedInfo);
        pFixedInfo = (FIXED_INFO *)::malloc(ulOutBufLen);
        if (pFixedInfo == nullptr) {
            qDebug() << "Error allocating memory needed to call GetNetworkParams";
            return QStringList();
        }
    }

    if ((ulRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen)) == NO_ERROR) {

        ret.append(pFixedInfo->DnsServerList.IpAddress.String);

        auto pIPAddr = pFixedInfo->DnsServerList.Next;
        while (pIPAddr) {
            ret.append(pFixedInfo->DnsServerList.IpAddress.String);
            pIPAddr = pIPAddr->Next;
        }
    }
    else {
        qDebug() << "GetNetworkParams failed with error: " << ulRetVal;
        return QStringList();
    }

    ::free(pFixedInfo);
    return ret;
} */

QString TunTap::getNameAndDefaultDNS(int ifInd) {
    if (ifInd < 0) {
        qCritical() << "[TunTap] Unable to operate the adapter with index [" << QString::number(ifInd) << ']';
        return "";
    }
    QString ret = ifMap[ifInd].adpName;
    if (!ret.isEmpty())
        return ret;
    u_long len = APIBUFLEN;
    PIP_ADAPTER_ADDRESSES adpAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(alloca(APIBUFLEN));
    u_long err;
    if ((err = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_GATEWAYS, nullptr, adpAddresses, &len)) != ERROR_SUCCESS) {
        qCritical() << "[TunTap] Can't store adapters info";
        return ret;
    }
    for (PIP_ADAPTER_ADDRESSES adpAddr = adpAddresses; adpAddr; adpAddr = adpAddr->Next) {
        if (adpAddr->IfIndex == ifInd) {
            ret = ifMap[ifInd].adpName = QString(adpAddr->AdapterName);
            for (PIP_ADAPTER_DNS_SERVER_ADDRESS &dnsAddr = adpAddr->FirstDnsServerAddress; dnsAddr; dnsAddr = adpAddr->FirstDnsServerAddress->Next) {
                IN_ADDR ipAddr = {
                    {
                        .S_addr = ((sockaddr_in*)(dnsAddr->Address.lpSockaddr))->sin_addr.S_un.S_addr
                    }
                };
                ifMap[ifInd].dnsAddrs.append(inet_ntoa(ipAddr));
                ifMap[ifInd].dnsAddrs.append(",");
            }
            break;
        }
    }
    ifMap[ifInd].dnsAddrs.chop(1);
    return ret;
}

int TunTap::write_tun(int devID, unsigned char *buf, size_t len)
{
    int lo_len = -1;
    bool status;
    int  err;
    status = WriteFile(
               tunDevice->_fileHandle,
               buf,
               (DWORD)len,
               &tunDevice->writes.size,
               &tunDevice->writes.overlapped
               );

    lo_len = tunDevice->writes.size;

    if (!status && GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(tunDevice->writes.overlapped.hEvent, INFINITE);
        err = GetOverlappedResult(tunDevice->_fileHandle, &tunDevice->writes.overlapped, (LPDWORD)&tunDevice->writes.size, FALSE);
        if (tunDevice->writes.size != len) {
            //doCloseTun();
            return -1;
        }
        if (err)
            return tunDevice->writes.size;

    }
    return lo_len;
}

int TunTap::read_tun(int devID, u_char *buf, size_t len) {
        HANDLE hEvents[2] = { tunDevice->reads.overlapped.hEvent, hInactive };
        if (!ReadFile(tunDevice->_fileHandle,
                      buf, (DWORD)len,
                      &tunDevice->reads.size,
                      &tunDevice->reads.overlapped))
        {
            if (ulong retCode = GetLastError() == ERROR_IO_PENDING) {
                goto READ_SUCCESS;
            } else {
                qCritical() << "[TunTap] error reading from tun: " << retCode;
                return -1;
            }
        }
READ_SUCCESS:
        DWORD err = WaitForMultipleObjects(2, hEvents, false, INFINITE);
        if (err == WAIT_OBJECT_0) {
            GetOverlappedResult(tunDevice->_fileHandle, &tunDevice->reads.overlapped, (LPDWORD)&tunDevice->reads.size, FALSE);
            return tunDevice->reads.size;
        } else
            return -3;
}

int TunTap::ipReleaseAddr(int ifInd) {
    PIP_INTERFACE_INFO pInfo = reinterpret_cast<PIP_INTERFACE_INFO>(alloca(APIBUFLEN));
    unsigned long ulOutBufLen = APIBUFLEN;
    unsigned long ulRetVal = 0;
    ulRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen);
    if (ulRetVal == NO_ERROR) {
        for (int i=0; i < pInfo->NumAdapters; ++i) {
            if (pInfo->Adapter[i].Index == ifInd) {
                if (IpReleaseAddress(&pInfo->Adapter[i]) == ERROR_SUCCESS) {
                    return 0;
                } else {
                    qCritical() << "Error: IP Config couldn't be released. " << GetLastErrorAsString();
                    return -1;
                }
            }
        }
        return 1;
    }
    qCritical() << "Error: couldn't get interfaces info. " << GetLastErrorAsString();
    return 2;
}

int TunTap::ipRenewAddr(int ifInd) {
    PIP_INTERFACE_INFO pInfo = reinterpret_cast<PIP_INTERFACE_INFO>(alloca(APIBUFLEN));
    unsigned long ulOutBufLen = APIBUFLEN;
    unsigned long ulRetVal = 0;
    ulRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen);
    if (ulRetVal == NO_ERROR) {
        for (int i=0; i < pInfo->NumAdapters; ++i) {
            if (pInfo->Adapter[i].Index == ifInd) {
                if (IpRenewAddress(&pInfo->Adapter[i]) == ERROR_SUCCESS) {
                    return 0;
                } else {
                    qDebug() << "Error: IP Config couldn't be renewed. " << GetLastErrorAsString();
                    return -1;
                }
            }
        }
        return 1;
    }
    qDebug() << "Error: couldn't get interfaces info. " << GetLastErrorAsString();
    return 2;
}

QString GetLastErrorAsString() {
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0)
        return QString("No error");

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    QString message = QString::fromLatin1(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

BOOL TunTap::setDNS(ulong ifIndex, QString sDNS) {
    HKEY hKey;
    QString strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\" + ifMap[ifIndex].adpName;

    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     strKeyName.toStdWString().c_str(),
                     0,
                     KEY_WRITE,
                     &hKey) != ERROR_SUCCESS )
        return false;
    if (!sDNS.isEmpty()) {
        TCHAR Buffer[64] = { };
        MultiByteToWideChar(CP_ACP, 0, qPrintable(sDNS), -1, Buffer, 256);
        RegSetValueEx(hKey, L"NameServer", 0, REG_SZ, reinterpret_cast<LPBYTE>(Buffer), sDNS.length() * sizeof(TCHAR));
    } else {
        RegDeleteValue(hKey, L"NameServer");
    }
    RegCloseKey(hKey);
    return true;
}

BOOL TunTap::resetDNS() {
    BOOL res = true;
    for (auto iter = ifMap.constBegin(), end = ifMap.constEnd(); iter != end; ++iter) {
        if (iter.key() == _ifIndex[1]) continue;
        if (!setDNS(iter.key(), ifMap[iter.key()].dnsAddrs)) res = false;
    }
    return res;
}

BOOL TunTap::notifyIPChange(QString sAdapterName) {
    BOOL bResult = FALSE;

    WCHAR wcAdapterName[256] = { };
    MultiByteToWideChar(CP_ACP, 0, qPrintable(sAdapterName), -1, wcAdapterName, 256);

    if((pDhcpNotifyProc)(nullptr, wcAdapterName, FALSE, 0, 0UL, 0UL, 0) == ERROR_SUCCESS) {
        bResult = TRUE;
    } else {
        qCritical() << "Couldn't update DHCP config: " << GetLastErrorAsString();
    }
    return bResult;
}

BOOL TunTap::flushDNS() {
    if (DnsFlushResolverCache)
        return DnsFlushResolverCache();
    else
        return FALSE;
}

bool TunTap::enableTapAdapter() {
    bool bResult = false;
    if (!NcFreeNetconProperties) {
        qCritical() << "Couldn't plug TAP adapter! Do it manually in Network Center.";
        return bResult;
    }
    INetConnectionManager   *pMngr;
    IEnumNetConnection      *pEnum;
    INetConnection          *pCon;
    CoInitialize(nullptr);
    CoCreateInstance(CLSID_ConnectionManager, 0, CLSCTX_ALL, IID_INetConnectionManager, (void**)&pMngr);
    pMngr->EnumConnections(NCME_DEFAULT, &pEnum);
    u_long c = 0;
    for ( ; (pEnum->Next(1, &pCon, &c) == S_OK) && (!bResult); ) {
        NETCON_PROPERTIES *pProps = nullptr;
        pCon->GetProperties(&pProps);
        if ((pProps->Status == NCS_DISCONNECTED) || (pProps->Status == NCS_DISCONNECTING))  {
            if (wcscmp(pProps->pszwName, getTapName()) == 0) {
                bResult = (pCon->Connect() == S_OK);
            }
        }
        NcFreeNetconProperties(pProps);
        pCon->Release();
    }
    pEnum->Release();
    pMngr->Release();
    CoUninitialize();
    return bResult;
}

QString TunTap::lookupHost(const QString &host, const QString &port) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    struct addrinfo hints, *res;
    int ret;
    char *hostAddr;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    ret = getaddrinfo(qPrintable(host), qPrintable(port), &hints, &res);
    if (ret != 0){
          return QString();
    }
    struct sockaddr_in *addr;
    addr = (struct sockaddr_in *)res->ai_addr;
    hostAddr = inet_ntoa((struct in_addr)addr->sin_addr);
    WSACleanup();
    freeaddrinfo(res);
    return QString(hostAddr);
}

TunTap::~TunTap() {
    FreeLibrary(hDnsApi);
    FreeLibrary(hNetShell);
    FreeLibrary(hDhcpDll);
}
