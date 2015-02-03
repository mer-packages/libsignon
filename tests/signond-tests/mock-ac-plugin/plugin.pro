include(../../tests.pri)

TEMPLATE = lib
TARGET = signon-mock-ac

CONFIG += \
    link_pkgconfig \
    plugin \
    qt

QT += \
    core \
    dbus
QT -= gui

DEFINES += \
    IDENTITY_AC_HELPER=\\\"$${TOP_BUILD_DIR}/tests/signond-tests/mock-ac-plugin/identity-ac-helper\\\"

CONFIG(enable-p2p) {
    DEFINES += ENABLE_P2P
    PKGCONFIG += dbus-1
}

QMAKE_CXXFLAGS += \
    -fvisibility=hidden

LIBS += -lsignon-extension
INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/signond

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

HEADERS = \
    access-control-manager.h \
    plugin.h

SOURCES = \
    access-control-manager.cpp \
    plugin.cpp
