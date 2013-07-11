TARGET = ssotestplugin

include( ../plugins.pri )

HEADERS += ssotestplugin.h

SOURCES += ssotestplugin.cpp

headers.files = $$HEADERS
INSTALLS += headers

