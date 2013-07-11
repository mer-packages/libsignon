include( ../../common-project-config.pri )
include( $${TOP_SRC_DIR}/common-installs-config.pri )
include( $${TOP_SRC_DIR}/common-vars.pri )

TEMPLATE = subdirs
SUBDIRS = SignOn

# Input
public_headers += \
    signoncommon.h

HEADERS = $$public_headers

INCLUDEPATH += . \
    $$TOP_SRC_DIR/include

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

OTHER_FILES = \
    com.google.code.AccountsSSO.SingleSignOn.AuthService.xml \
    com.google.code.AccountsSSO.SingleSignOn.AuthSession.xml \
    com.google.code.AccountsSSO.SingleSignOn.Identity.xml

headers.files = $$public_headers
headers.path = $${INSTALL_PREFIX}/include/signond

dbus_files.files = $$OTHER_FILES
dbus_files.path =$${INSTALL_PREFIX}/share/dbus-1/interfaces

pkgconfig.files = signond.pc
include($${TOP_SRC_DIR}/common-pkgconfig.pri)

INSTALLS += headers dbus_files pkgconfig

