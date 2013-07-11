
TEMPLATE = lib
TARGET = exampleplugin
DESTDIR = lib/signon
QT += core

CONFIG += plugin \
        build_all \
        warn_on \
        link_pkgconfig

HEADERS += exampleplugin.h \
    exampledata.h

SOURCES += exampleplugin.cpp

INCLUDEPATH += . \
    /usr/include/signon-qt \
    /usr/include/signon-plugins

QMAKE_CLEAN += libexample.so
headers.files = $$HEADERS

target.path  = $${INSTALL_PREFIX}/lib/signon
INSTALLS = target
