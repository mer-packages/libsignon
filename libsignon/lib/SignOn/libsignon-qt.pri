include( ../../common-project-config.pri )
include( ../../common-vars.pri )
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET = signon-qt5
} else {
    TARGET = signon-qt
}

# Input
public_headers += \
    sessiondata.h \
    identityinfo.h \
    authservice.h \
    identity.h \
    authsession.h \
    libsignoncommon.h \
    signon.h \
    signonerror.h

private_headers = authserviceimpl.h \
    identityimpl.h \
    authsessionimpl.h \
    identityinfoimpl.h \
    dbusoperationqueuehandler.h \
    dbusinterface.h

HEADERS = $$public_headers \
    $$private_headers

INCLUDEPATH += . \
    $$TOP_SRC_DIR/include

SOURCES += identityinfo.cpp \
    identity.cpp \
    identityimpl.cpp \
    authservice.cpp \
    authserviceimpl.cpp \
    authsession.cpp \
    authsessionimpl.cpp \
    identityinfoimpl.cpp \
    dbusoperationqueuehandler.cpp \
    dbusinterface.cpp

QT += core \
    dbus

CONFIG += \
    build_all \
    link_pkgconfig
    
DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_NO_CAST_FROM_ASCII \
    LIBSIGNON_TRACE

include( $$TOP_SRC_DIR/common-installs-config.pri )

headers.files = $$public_headers \
    AuthService \
    AuthSession \
    Error \
    Identity \
    IdentityInfo \
    SessionData
headers.path = $${INSTALL_PREFIX}/include/$${TARGET}/SignOn
INSTALLS += headers

pkgconfig.files = lib$${TARGET}.pc
include($${TOP_SRC_DIR}/common-pkgconfig.pri)
INSTALLS += pkgconfig
