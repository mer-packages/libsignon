include( ../tests.pri )
TARGET = signon-passwordplugin-tests
QT += core \
    network \
    testlib
CONFIG += \
    link_pkgconfig

DEFINES += SIGNON_PLUGIN_TRACE

SOURCES += passwordplugintest.cpp

HEADERS += passwordplugintest.h \
    $${TOP_SRC_DIR}/src/plugins/password/passwordplugin.h \
    $${TOP_SRC_DIR}/lib/plugins/SignOn/authpluginif.h

INCLUDEPATH += $${TOP_SRC_DIR}/lib/plugins \
    $${TOP_SRC_DIR}/src/plugins/password \
    $${TOP_SRC_DIR}/src/plugins

QMAKE_CXXFLAGS += -fno-exceptions \
    -fno-rtti
