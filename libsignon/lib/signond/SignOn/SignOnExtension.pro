include( ../../../common-project-config.pri )
include( $${TOP_SRC_DIR}/common-vars.pri )

TEMPLATE = lib
TARGET = signon-extension

HEADERS = \
    abstract-access-control-manager.h \
    abstract-crypto-manager.h \
    abstract-key-authorizer.h \
    abstract-key-manager.h \
    abstract-secrets-storage.h \
    debug.h \
    export.h \
    extension-interface.h \
    key-handler.h \
    misc.h

INCLUDEPATH += \
    ..

SOURCES += \
    abstract-access-control-manager.cpp \
    abstract-crypto-manager.cpp \
    abstract-key-authorizer.cpp \
    abstract-key-manager.cpp \
    abstract-secrets-storage.cpp \
    debug.cpp \
    extension-interface.cpp \
    key-handler.cpp \
    misc.cpp

QT += core \
      dbus

QMAKE_CXXFLAGS += \
    -fvisibility=hidden

# Error on undefined symbols
QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
DEFINES += \
    BUILDING_SIGNON \
    SIGNON_ENABLE_UNSTABLE_APIS \
    SIGNON_TRACE

include( $${TOP_SRC_DIR}/common-installs-config.pri )

headers.files = \
    AbstractAccessControlManager \
    abstract-access-control-manager.h \
    AbstractCryptoManager \
    abstract-crypto-manager.h \
    AbstractKeyAuthorizer \
    abstract-key-authorizer.h \
    AbstractKeyManager \
    abstract-key-manager.h \
    AbstractSecretsStorage \
    abstract-secrets-storage.h \
    Debug \
    debug.h \
    export.h \
    ExtensionInterface \
    extension-interface.h \
    KeyHandler \
    key-handler.h
headers.path = $${INSTALL_PREFIX}/include/$${TARGET}/SignOn
INSTALLS += headers

pkgconfig.files = SignOnExtension.pc
include($${TOP_SRC_DIR}/common-pkgconfig.pri)

INSTALLS += pkgconfig
