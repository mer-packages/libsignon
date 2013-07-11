include( common-vars.pri )

TEMPLATE  = subdirs
CONFIG   += ordered
SUBDIRS   = lib src server tests

include( common-installs-config.pri )

include( doc/doc.pri )

DISTNAME = $${PROJECT_NAME}-$${PROJECT_VERSION}
dist.commands = "git archive --format=tar --prefix=$${DISTNAME}/ HEAD | bzip2 -9 > $${DISTNAME}.tar.bz2"
QMAKE_EXTRA_TARGETS += dist
# End of File
