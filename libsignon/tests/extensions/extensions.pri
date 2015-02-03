include(../tests.pri)

QT += core \
    sql \
    testlib \
    dbus

QT -= gui

LIBS += -lsignon-extension

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

DEFINES += SIGNOND_TRACE
DEFINES += SIGNON_PLUGIN_TRACE

INCLUDEPATH += . \
    $$TOP_SRC_DIR/lib/signond

QMAKE_CXXFLAGS += \
    -fno-exceptions \
    -fno-rtti
