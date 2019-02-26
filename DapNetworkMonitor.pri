SOURCES += \
    $$PWD/DapNetworkMonitorAbstract.cpp \
    $$PWD/DapMonitorCmdProgram.cpp
HEADERS += \
    $$PWD/DapNetworkMonitor.h \
    $$PWD/DapNetworkMonitorAbstract.h \
    $$PWD/DapMonitorCmdProgram.h

win32 {
SOURCES += $$PWD/DapNetworkMonitorWindows.cpp
HEADERS += $$PWD/DapNetworkMonitorWindows.h
}

linux-*: !android {
SOURCES += $$PWD/DapNetworkMonitorLinux.cpp
HEADERS += $$PWD/DapNetworkMonitorLinux.h
}

android {
SOURCES += $$PWD/DapNetworkMonitorAndroid.cpp
HEADERS += $$PWD/DapNetworkMonitorAndroid.h
}

macos {
LIBS += -framework Foundation
LIBS += -framework CoreFoundation
#LIBS += -framework NetworkExtension
LIBS += -framework SystemConfiguration
SOURCES += $$PWD/DapNetworkMonitorDarwin.cpp
HEADERS += $$PWD/DapNetworkMonitorDarwin.h
}

INCLUDEPATH += $$PWD
