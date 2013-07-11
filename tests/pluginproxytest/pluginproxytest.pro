include( ../tests.pri )

CONFIG += \
    link_pkgconfig

QT += \
    core \
    dbus \
    testlib
QT -= gui

LIBS += \
    -lsignon-extension

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

DEFINES += SIGNOND_TRACE
DEFINES += SIGNON_PLUGIN_TRACE

SOURCES = \
    testpluginproxy.cpp \
    include.cpp

HEADERS += testpluginproxy.h \
           $${TOP_SRC_DIR}/src/signond/pluginproxy.h \
           $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn/blobiohandler.h \
           $${TOP_SRC_DIR}/lib/plugins/SignOn/authpluginif.h

TARGET = testpluginproxy

INCLUDEPATH += . \
    $${TOP_SRC_DIR}/lib/plugins \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn \
    $${TOP_SRC_DIR}/lib/signond \
    $${TOP_SRC_DIR}/src/plugins \
    $${TOP_SRC_DIR}/src/signond

