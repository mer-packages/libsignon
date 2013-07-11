include( ../plugins.pri )

TARGET = exampleplugin

HEADERS += exampleplugin.h \
    exampledata.h

SOURCES += exampleplugin.cpp

headers.files = $$HEADERS
INSTALLS += headers

example.path = $${INSTALL_PREFIX}/share/doc/signon-plugins-dev/example
example.files = exampleplugin.h \
    exampleplugin.cpp \
    exampleplugin.pro \
    exampledata.h
INSTALLS += example
