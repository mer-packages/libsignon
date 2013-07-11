include( ../tests.pri )

CONFIG += \
    link_pkgconfig

QT += core \
    sql \
    testlib \
    xml \
    network \
    dbus

QT -= gui

LIBS += -lsignon-extension
greaterThan(QT_MAJOR_VERSION, 4) {
    LIBS += -lsignon-qt5
} else {
    LIBS += -lsignon-qt
}

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

#DEFINES += CAM_UNIT_TESTS_FIXED

DEFINES += SIGNOND_TRACE
DEFINES += SIGNON_PLUGIN_TRACE

HEADERS += \
    timeouts.h \
    $$TOP_SRC_DIR/src/signond/pluginproxy.h \
    $$TOP_SRC_DIR/tests/pluginproxytest/testpluginproxy.h \
    backuptest.h \
    databasetest.h \
    $$TOP_SRC_DIR/src/signond/credentialsdb.h \
    $$TOP_SRC_DIR/src/signond/default-secrets-storage.h \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn/blobiohandler.h

SOURCES = \
    signond-tests.cpp \
    timeouts.cpp \
    $$TOP_SRC_DIR/tests/pluginproxytest/testpluginproxy.cpp \
    $$TOP_SRC_DIR/tests/pluginproxytest/include.cpp \
    backuptest.cpp \
    databasetest.cpp \
    $$TOP_SRC_DIR/src/signond/credentialsdb.cpp \
    $$TOP_SRC_DIR/src/signond/default-secrets-storage.cpp

contains(DEFINES, CAM_UNIT_TESTS_FIXED) {
 HEADERS *=$$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server/credentialsaccessmanagertest.h \
           $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server/dbuspeer.h \
           $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/defs.h \
           $$TOP_SRC_DIR/src/signond/credentialsaccessmanager.h \
           $$TOP_SRC_DIR/src/signond/accesscodehandler.h \
           $$TOP_SRC_DIR/src/signond/simdbusadaptor.h \
           $$TOP_SRC_DIR/src/signond/cryptomanager.h \
           $$TOP_SRC_DIR/src/signond/credentialsdb.h \
           $$TOP_SRC_DIR/src/signond/credentialsdb_p.h

 SOURCES *= $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server/credentialsaccessmanagertest.cpp \
            $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server/includes.cpp \
            $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server/dbuspeer.cpp

 LIBS += -lcreds
}

TARGET = signon-tests

INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/plugins \
    $$TOP_SRC_DIR/lib/signond \
    $$TOP_SRC_DIR/tests/pluginproxytest \
    $$TOP_SRC_DIR/src/signond \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn \
    $$TOP_SRC_DIR/tests/credentialsaccessmanagertest/cam-test-server

DEFINES += SSO_CI_TESTMANAGEMENT

QMAKE_CXXFLAGS += -fno-exceptions \
    -fno-rtti

check.depends = $$TARGET
check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/non-existing-dir $$RUN_WITH_SIGNOND ./signon-tests"
