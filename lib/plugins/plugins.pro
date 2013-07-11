TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = signon-plugins.pro signon-plugins-common

TARGET=signon-plugins
include(doc/doc.pri)

