TARGET = tst_pluginproxy

include(signond-tests.pri)

HEADERS += \
    testpluginproxy.h \
    $$TOP_SRC_DIR/src/signond/pluginproxy.h \
    $${TOP_SRC_DIR}/lib/plugins/signon-plugins-common/SignOn/blobiohandler.h

SOURCES = \
    testpluginproxy.cpp \
    include.cpp
