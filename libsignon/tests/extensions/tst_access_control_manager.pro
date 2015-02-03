include(extensions.pri)

TARGET = tst_access_control_manager

SOURCES += \
    tst_access_control_manager.cpp

check.depends = $$TARGET
check.commands = "./$$TARGET"
