include( ../../common-project-config.pri )
include( ../../common-vars.pri )
TEMPLATE = app
TARGET = signonpluginprocess
QT += core network
QT -= gui

HEADERS += \
    debug.h \
    remotepluginprocess.h

SOURCES += \
    debug.cpp \
    main.cpp \
    remotepluginprocess.cpp

INCLUDEPATH += . \
               $$TOP_SRC_DIR/src \
               $$TOP_SRC_DIR/src/plugins \
               $$TOP_SRC_DIR/src/signond \
               $$TOP_SRC_DIR/lib/plugins/signon-plugins-common \
               $$TOP_SRC_DIR/lib/plugins

CONFIG += \
    build_all \
    link_pkgconfig

system(pkg-config --exists libproxy-1.0) {
    DEFINES += HAVE_LIBPROXY
    PKGCONFIG += libproxy-1.0
    SOURCES += my-network-proxy-factory.cpp
}

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/plugins/signon-plugins-common

LIBS += -lsignon-plugins-common

QMAKE_CXXFLAGS += -fno-exceptions \
                  -fno-rtti

#DEFINES += QT_NO_CAST_TO_ASCII \
#    QT_NO_CAST_FROM_ASCII
DEFINES += SIGNON_PLUGIN_TRACE
DEFINES += "SIGNOND_PLUGINS_DIR=$${SIGNOND_PLUGINS_DIR_QUOTED}"

include( ../../common-installs-config.pri )
