# Include this file after defining the pkgconfig.files variable

!isEmpty(pkgconfig.files) {
    QMAKE_SUBSTITUTES += $${pkgconfig.files}.in
    pkgconfig.CONFIG = no_check_exist
    pkgconfig.path  = $${INSTALL_LIBDIR}/pkgconfig
    QMAKE_EXTRA_TARGETS += pkgconfig

    QMAKE_CLEAN += $${pkgconfig.files}
}
