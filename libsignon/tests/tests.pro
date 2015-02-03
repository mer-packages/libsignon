include( ../common-project-config.pri )
include( ../common-vars.pri )

TEMPLATE = subdirs

CONFIG  += ordered

SUBDIRS += create-tests-definition.pro

SUBDIRS += \
    passwordplugintest \
    libsignon-qt-tests \
    signond-tests \
    extensions

QMAKE_SUBSTITUTES += com.google.code.AccountsSSO.SingleSignOn.service.in

# copy testsession.conf to the build directory
system($$QMAKE_COPY testsession.conf.in $${TOP_BUILD_DIR}/tests/testsession.conf)

QMAKE_DISTCLEAN += \
    testsession.conf
