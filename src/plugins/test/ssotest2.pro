TARGET = ssotest2plugin

include( ../plugins.pri )

QT += gui

HEADERS += ssotest2plugin.h \
           ssotest2data.h

SOURCES += ssotest2plugin.cpp

headers.files = $$HEADERS
INSTALLS += headers

RESOURCES += captcha-images.qrc

