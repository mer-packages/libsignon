TARGET = passwordplugin

include( ../plugins.pri )

HEADERS += passwordplugin.h

SOURCES += passwordplugin.cpp

headers.files = $$HEADERS
INSTALLS += headers

