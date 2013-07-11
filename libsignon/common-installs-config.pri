#-----------------------------------------------------------------------------
# Common installation configuration for all projects.
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# default installation target for applications
#-----------------------------------------------------------------------------
contains( TEMPLATE, app ) {
    target.path  = $${INSTALL_PREFIX}/bin
    INSTALLS    += target
    message("====")
    message("==== INSTALLS += target")
}


#-----------------------------------------------------------------------------
# default installation target for libraries
#-----------------------------------------------------------------------------
contains( TEMPLATE, lib ) {
    target.path  = $${INSTALL_LIBDIR}
    INSTALLS    += target
    message("====")
    message("==== INSTALLS += target")
}

#-----------------------------------------------------------------------------
# target for header files
#-----------------------------------------------------------------------------
!isEmpty( headers.files ) {
    headers.path  = $${INSTALL_PREFIX}/include/$${TARGET}
    INSTALLS     += headers
    message("====")
    message("==== INSTALLS += headers")
} else {
    message("====")
    message("==== NOTE: Remember to add your API headers into `headers.files' for installation!")
}


# End of File
