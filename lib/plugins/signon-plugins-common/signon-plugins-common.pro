TEMPLATE = lib
TARGET = signon-plugins-common

include( ../../../common-project-config.pri )
include($${TOP_SRC_DIR}/common-installs-config.pri)
include($${TOP_SRC_DIR}/common-vars.pri)

CONFIG += qt
QT += dbus

INCLUDEPATH += ../

DEFINES += SIGNON_PLUGIN_TRACE

SOURCES += \
    SignOn/blobiohandler.cpp
HEADERS += \
    SignOn/blobiohandler.h \
    SignOn/ipc.h

headers.files = \
    SignOn/blobiohandler.h

headers.path = $${INSTALL_PREFIX}/include/signon-plugins/SignOn
INSTALLS += headers

pkgconfig.files = signon-plugins-common.pc
include($${TOP_SRC_DIR}/common-pkgconfig.pri)
INSTALLS += pkgconfig
