#include "DapNetworkMonitorIOS.h"

DapNetworkMonitorIOS::DapNetworkMonitorIOS(QObject *parent):
    DapNetworkMonitorAbstract(parent) {

}

bool DapNetworkMonitorIOS::isTunDriverInstalled() const {
    /* Todo */
    return true;
}

bool DapNetworkMonitorIOS::monitoringStart() {
    /* Todo */
    return false;
}

bool DapNetworkMonitorIOS::monitoringStop() {
    /* Todo */
    return false;
}
