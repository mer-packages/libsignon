include( libsignon-qt-tests.pri )

TARGET = libsignon-qt-untrusted-tests
SOURCES += libsignon-qt-untrusted-tests.cpp
SOURCES -= $$TOP_SRC_DIR/tests/sso-mt-test/ssotestclient.cpp

check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/non-existing-dir $$RUN_WITH_SIGNOND ./libsignon-qt-tests"
