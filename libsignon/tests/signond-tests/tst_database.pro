TARGET = tst_database

include(signond-tests.pri)

HEADERS += \
    databasetest.h \
    $$TOP_SRC_DIR/src/signond/credentialsdb.h \
    $$TOP_SRC_DIR/src/signond/default-secrets-storage.h

SOURCES = \
    databasetest.cpp \
    $$TOP_SRC_DIR/src/signond/credentialsdb.cpp \
    $$TOP_SRC_DIR/src/signond/default-secrets-storage.cpp
