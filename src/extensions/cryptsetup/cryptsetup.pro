include( ../../../common-project-config.pri )
include( $${TOP_SRC_DIR}/common-vars.pri )

TEMPLATE = lib
TARGET = cryptsetup

CONFIG += \
    plugin \
    qt

HEADERS = \
    crypto-handlers.h \
    crypto-manager.h \
    cryptsetup-plugin.h \
    debug.h \
    misc.h

INCLUDEPATH += \
    $${TOP_SRC_DIR}/lib/signond

SOURCES += \
    crypto-handlers.cpp \
    crypto-manager.cpp \
    cryptsetup-plugin.cpp \
    misc.cpp

QT += core
QT -= gui

QMAKE_CXXFLAGS += \
    -fno-exceptions \
    -fno-rtti \
    -fvisibility=hidden

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
DEFINES += \
    SIGNON_TRACE

LIBS += \
    -lcryptsetup

include( $${TOP_SRC_DIR}/common-installs-config.pri )

target.path  = $${INSTALL_LIBDIR}/signon/extensions
INSTALLS    += target

