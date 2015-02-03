TARGET = tst_access_control

include(signond-tests.pri)

SOURCES = \
    tst_access_control.cpp

check.commands = "SSO_PLUGINS_DIR=$${TOP_BUILD_DIR}/src/plugins/test SSO_EXTENSIONS_DIR=$${TOP_BUILD_DIR}/tests/signond-tests/mock-ac-plugin SSO_CONFIG_FILE_DIR=$${TOP_SRC_DIR}/tests/signond-tests/mock-ac-plugin $$RUN_WITH_SIGNOND ./$$TARGET"
