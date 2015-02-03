include(../../common-project-config.pri)

TEMPLATE = app
TARGET = identity-tool

QT += core
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4) {
    LIBS += -lsignon-qt5
} else {
    LIBS += -lsignon-qt
}

QMAKE_LIBDIR += \
    $${TOP_BUILD_DIR}/lib/signond/SignOn
QMAKE_RPATHDIR = $${QMAKE_LIBDIR}

INCLUDEPATH += \
    $${TOP_SRC_DIR}/lib

SOURCES = \
    identity-tool.cpp
