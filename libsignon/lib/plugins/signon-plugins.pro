TEMPLATE = lib
TARGET = signon-plugins

include( ../../common-project-config.pri )
include($${TOP_SRC_DIR}/common-installs-config.pri)
include($${TOP_SRC_DIR}/common-vars.pri)

HEADERS = \
    SignOn/authpluginif.h \
    SignOn/uisessiondata.h \
    SignOn/uisessiondata_priv.h

headers.files = \
    SignOn/AuthPluginInterface \
    SignOn/authpluginif.h \
    SignOn/signonplugincommon.h \
    SignOn/UiSessionData \
    SignOn/uisessiondata.h \
    SignOn/uisessiondata_priv.h
headers.path = $${INSTALL_PREFIX}/include/signon-plugins/SignOn
INSTALLS += headers

pkgconfig.files = signon-plugins.pc
include($${TOP_SRC_DIR}/common-pkgconfig.pri)
INSTALLS += pkgconfig
