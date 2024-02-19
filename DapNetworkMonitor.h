#ifndef DAPNETWORKMONITOR_H
#define DAPNETWORKMONITOR_H

#include <QtGlobal>
#include <QtSystemDetection>

#if defined(Q_OS_ANDROID)
#include "DapNetworkMonitorAndroid.h"
typedef class DapNetworkMonitorAndroid DapNetworkMonitor;
#elif defined(Q_OS_LINUX)
#include "DapNetworkMonitorLinux.h"
typedef class DapNetworkMonitorLinux DapNetworkMonitor;
#elif defined(Q_OS_WIN)
#include "DapNetworkMonitorWindows.h"
typedef class DapNetworkMonitorWindows DapNetworkMonitor;
#elif defined(Q_OS_MACOS)
#include "DapNetworkMonitorDarwin.h"
typedef class DapNetworkMonitorDarwin DapNetworkMonitor;
#elif defined(Q_OS_IOS)
#include "DapNetworkMonitorIOS.h"
typedef class DapNetworkMonitorIOS DapNetworkMonitor;
#endif

#endif // DAPNETWORKMONITOR_H
