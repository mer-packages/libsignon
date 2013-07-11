include( ../../../common-project-config.pri )
include( $${TOP_SRC_DIR}/common-vars.pri )

TEMPLATE = lib
TARGET = sso-mssf-ac

CONFIG += \
    plugin \
    qt \
    mssf-qt

MSSF += creds

HEADERS = \
    mssf-access-control-manager.h \
    mssfac-plugin.h \
    debug.h 

INCLUDEPATH += \
    $${TOP_SRC_DIR}/lib/signond

SOURCES += \
    mssf-access-control-manager.cpp \
    mssfac-plugin.cpp 

QT += core \
      dbus \
      mssf-qt
QT -= gui

QMAKE_CXXFLAGS += \
    -fno-exceptions \
    -fno-rtti \
    -fvisibility=hidden

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
DEFINES += \
    SIGNON_TRACE

include( $${TOP_SRC_DIR}/common-installs-config.pri )

target.path  = $${INSTALL_LIBDIR}/signon/extensions
INSTALLS    += target
