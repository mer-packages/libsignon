#-----------------------------------------------------------------------------
# Common configuration for all projects.
#-----------------------------------------------------------------------------

QT		-= gui
CONFIG         += link_pkgconfig
#MOC_DIR         = .moc
#OBJECTS_DIR     = .obj
RCC_DIR         = resources
#UI_DIR          = ui
#UI_HEADERS_DIR  = ui/include
#UI_SOURCES_DIR  = ui/src

# we don't like warnings...
QMAKE_CXXFLAGS -= -Werror -Wno-write-strings
# Disable RTTI
QMAKE_CXXFLAGS += -fno-exceptions -fno-rtti

greaterThan(QT_MAJOR_VERSION, 4) {
    # Qt5: use C++0x. This is used to avoid the source incompatibility
    # with the QSKIP macro, as described in:
    # http://www.kdab.com/porting-from-qt-4-to-qt-5/
    QMAKE_CXXFLAGS += -std=c++0x
}

TOP_SRC_DIR     = $$PWD
TOP_BUILD_DIR   = $${TOP_SRC_DIR}/$${BUILD_DIR}
QMAKE_LIBDIR   += $${TOP_BUILD_DIR}/lib/SignOn
INCLUDEPATH    += $${TOP_SRC_DIR}/lib

#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += DEBUG_ENABLED
#TODO comment this to restrict plugins to run under signon user
DEFINES += NO_SIGNON_USER

# Qt4/Qt5 common checks
greaterThan(QT_MAJOR_VERSION, 4) {
    LIBSIGNON = libsignon-qt5
    CMAKE_BASENAME = SignOnQt5
    LIBQTCORE = Qt5Core
} else {
    LIBSIGNON = libsignon-qt
    CMAKE_BASENAME = SignOnQt
    LIBQTCORE = QtCore
}

#-----------------------------------------------------------------------------
# setup the installation prefix
#-----------------------------------------------------------------------------
INSTALL_PREFIX = /usr  # default installation prefix

# default prefix can be overriden by defining PREFIX when running qmake
isEmpty( PREFIX ) {
    message("====")
    message("==== NOTE: To override the installation path run: `qmake PREFIX=/custom/path'")
    message("==== (current installation path is `$${INSTALL_PREFIX}')")
} else {
    INSTALL_PREFIX = $${PREFIX}
    message("====")
    message("==== install prefix set to `$${INSTALL_PREFIX}'")
}

# Setup the library installation directory
exists( meego-release ) {
    ARCH = $$system(tail -n1 meego-release)
} else {
    ARCH = $$system(uname -m)
}

INSTALL_LIBDIR = $${INSTALL_PREFIX}/lib

# default library directory can be overriden by defining LIBDIR when
# running qmake
isEmpty( LIBDIR ) {
    message("====")
    message("==== NOTE: To override the library installation path run: `qmake LIBDIR=/custom/path'")
    message("==== (current installation path is `$${INSTALL_LIBDIR}')")
} else {
    INSTALL_LIBDIR = $${LIBDIR}
    message("====")
    message("==== library install path set to `$${INSTALL_LIBDIR}'")
}

isEmpty ( CMAKE_CONFIG_PATH ) {
    CMAKE_CONFIG_PATH = $${INSTALL_LIBDIR}/cmake/$${CMAKE_BASENAME}
    message("====")
    message("==== NOTE: To override the cmake module installation path run: `qmake CMAKE_CONFIG_PATH=/custom/path'")
    message("==== (current installation path is `$${CMAKE_CONFIG_PATH}')")
} else {
    message("====")
    message("==== cmake module install path set to `$${CMAKE_CONFIG_PATH}'")
}

# Default directory for signond extensions
_EXTENSIONS = $$(SIGNOND_EXTENSIONS_DIR)
isEmpty(_EXTENSIONS) {
    SIGNOND_EXTENSIONS_DIR = $${INSTALL_LIBDIR}/signon/extensions
} else {
    SIGNOND_EXTENSIONS_DIR = $$_EXTENSIONS
}
SIGNOND_EXTENSIONS_DIR_QUOTED = \\\"$$SIGNOND_EXTENSIONS_DIR\\\"

_PLUGINS = $$(SIGNOND_PLUGINS_DIR)
isEmpty(_PLUGINS) {
    SIGNOND_PLUGINS_DIR = $${INSTALL_LIBDIR}/signon
} else {
    SIGNOND_PLUGINS_DIR = $$_PLUGINS
}
SIGNOND_PLUGINS_DIR_QUOTED = \\\"$$SIGNOND_PLUGINS_DIR\\\"

# Note that you have to CONFIG+=install_tests in order to install tests
isEmpty(TESTDIR) {
    INSTALL_TESTDIR = $${INSTALL_LIBDIR}/signon
} else {
    INSTALL_TESTDIR = $${TESTDIR}
}

include( coverage.pri )
