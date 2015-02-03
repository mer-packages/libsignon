include( ../tests.pri )

TARGET = libsignon-qt-tests

CONFIG += \
    build_all \
    link_pkgconfig
QT += \
    core \
    dbus \
    testlib
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4) {
    LIBS *= -lsignon-qt5
} else {
    LIBS *= -lsignon-qt
}
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

SOURCES += \
    testauthsession.cpp \
    testthread.cpp \
    signon-ui.cpp \
    ssotestclient.cpp \
    testauthserviceresult.cpp \
    testidentityresult.cpp
HEADERS += \
    testauthsession.h \
    testthread.h \
    signon-ui.h \
    ssotestclient.h \
    testauthserviceresult.h \
    testidentityresult.h \
    $$TOP_SRC_DIR/src/plugins/test/ssotest2data.h
INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/plugins \
    $$TOP_SRC_DIR/src/plugins/test
DEFINES += SSO_CI_TESTMANAGEMENT
DEFINES += "SIGNOND_PLUGINS_DIR=$${SIGNOND_PLUGINS_DIR_QUOTED}"
QMAKE_CXXFLAGS += -fno-exceptions \
    -fno-rtti

check.depends = $$TARGET
check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/non-existing-dir $$RUN_WITH_SIGNOND ./libsignon-qt-tests"
