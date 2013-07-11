TEMPLATE = subdirs
SUBDIRS =

CONFIG(cryptsetup) {
    SUBDIRS += cryptsetup
}

system(pkg-config --exists mssf-qt) {
    SUBDIRS += mssf-ac
}

system(pkg-config --exists smack-qt) {
    SUBDIRS += smack-ac
}
