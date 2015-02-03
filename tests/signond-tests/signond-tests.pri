include(../tests.pri)

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

DEFINES += SIGNOND_TRACE
DEFINES += SIGNON_PLUGIN_TRACE

INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/plugins \
    $$TOP_SRC_DIR/lib/signond \
    $$TOP_SRC_DIR/src/signond \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn

QMAKE_CXXFLAGS += -fno-exceptions \
    -fno-rtti

check.depends = $$TARGET
check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/non-existing-dir $$RUN_WITH_SIGNOND ./$$TARGET"
