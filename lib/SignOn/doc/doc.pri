#-----------------------------------------------------------------------------
# doc.pri
#-----------------------------------------------------------------------------

# list of documentation folders to install
DOC_FOLDERS = doc/html \
              doc/qch

# files and folders listed in the installation target's .files section
# must exist _before_ qmake generates the Makefile...so, make sure our
# documentation target folders exist in the current build folder
for( folder, DOC_FOLDERS ) {
    system( mkdir -p $$(PWD)/$${folder} )
}


#-----------------------------------------------------------------------------
# extra build targets for generating and cleaning documentation
#-----------------------------------------------------------------------------
DOC_INPUT += $${_PRO_FILE_PWD_}
DOC_INPUT += doc/src

# target for generating documentation
doctarget.target     = docs
doctarget.commands   = OUTPUT_DIRECTORY=doc \
                       PROJECT_NAME=\"$${PROJECT_NAME}\" \
                       PROJECT_NUMBER=\"$${PROJECT_VERSION}\" \
                       STRIP_FROM_PATH=\"$${_PRO_FILE_PWD_}\" \
                       INPUT=\"$${DOC_INPUT}\" \
                       QHP_NAMESPACE=\"com.nokia.example.$${TARGET}\" \
                       QHP_VIRTUAL_FOLDER=\"$${TARGET}\" \
                       TAGFILES=\"$$system(pwd)/qt.tags\" \
                       TAGFILE=\"doc/$${TARGET}.tags\" \
                       doxygen $$system(pwd)/doxy.conf
doctarget.depends    = FORCE
QMAKE_EXTRA_TARGETS += doctarget


# target for cleaning generated documentation
doccleantarget.target = cleandocs
for( folder, DOC_FOLDERS ) {
    doccleantarget.commands += rm -r -f $${folder};
}
doccleantarget.commands += rm -r -f doc/accounts.tags;
doccleantarget.depends   = FORCE
QMAKE_EXTRA_TARGETS     += doccleantarget


#-----------------------------------------------------------------------------
# installation setup
# NOTE: remember to set headers.files before this include to have the headers
# properly setup.
#-----------------------------------------------------------------------------
include( ../../../common-project-config.pri )
include( ../../../common-installs-config.pri )


#-----------------------------------------------------------------------------
# Installation target setup for documentation
#-----------------------------------------------------------------------------
documentation.path = $${INSTALL_PREFIX}/share/doc/libsignon-qt
for( folder, DOC_FOLDERS ) {
    documentation.files += $${folder}
}
# make sure docs are generated before trying to install anything
documentation.depends  = docs
INSTALLS              += documentation
message("====")
message("==== INSTALLS += documentation")


# End of File

