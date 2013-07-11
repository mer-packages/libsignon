include( ../../../common-project-config.pri )
include( $${TOP_SRC_DIR}/common-vars.pri )

TEMPLATE = lib
TARGET = sso-smack-ac

CONFIG += \
    plugin \
    qt \
    smack-qt

HEADERS = \
    smack-access-control-manager.h \
    smackac-plugin.h \
    debug.h 

INCLUDEPATH += \
    $${TOP_SRC_DIR}/lib/signond

SOURCES += \
    smack-access-control-manager.cpp \
    smackac-plugin.cpp 

QT += core \
      dbus
QT -= gui

QMAKE_CXXFLAGS += \
    -fno-exceptions \
    -fno-rtti \
    -fvisibility=hidden

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
DEFINES += \
    SIGNON_TRACE

LIBS += \
    -lsmack-qt

include( $${TOP_SRC_DIR}/common-installs-config.pri )

target.path  = $${INSTALL_LIBDIR}/signon/extensions
INSTALLS    += target
