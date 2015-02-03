include( ../common-project-config.pri )
include( ../common-vars.pri )

include( ../common-installs-config.pri )

TEMPLATE = subdirs

QMAKE_SUBSTITUTES += com.google.code.AccountsSSO.SingleSignOn.service.in
QMAKE_SUBSTITUTES += com.nokia.SingleSignOn.Backup.service.in

service.path = $${INSTALL_PREFIX}/share/dbus-1/services
service.files = com.google.code.AccountsSSO.SingleSignOn.service
service.files += com.nokia.SingleSignOn.Backup.service
INSTALLS += service
