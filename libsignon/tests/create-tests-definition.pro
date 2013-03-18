include( tests.pri )

# added by default in tests.pri
INSTALLS -= target

# dummy
TEMPLATE = subdirs

create_tests_definition.target = tests.xml
create_tests_definition.commands = \
        $${PWD}/create-tests-definition.sh $${INSTALL_TESTDIR} > tests.xml
create_tests_definition.CONFIG = phony

make_default.CONFIG = phony
make_default.depends += create_tests_definition

QMAKE_EXTRA_TARGETS += create_tests_definition make_default

tests_definition.path = $${INSTALL_TESTDIR}
tests_definition.files = tests.xml
tests_definition.depends = tests.xml
tests_definition.CONFIG += no_check_exist
INSTALLS += tests_definition
