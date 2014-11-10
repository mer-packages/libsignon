#!/bin/sh

test_flag()
{
    FLAGS="${1?}"
    FLAG="${2?}"
    echo "${FLAGS}" |tr ',' '\n' |grep -q --line-regexp -e "${FLAG}"
}

fmt_test_case()
{
    EXECUTABLE="${1?}"
    F_NAME="${2?}"
    CASE_TYPE="${3?}"
    CASE_LEVEL="${4?}"

    EXEC_BASE_NAME="`basename "${EXECUTABLE}"`"

    cat <<END
      <case name="${EXEC_BASE_NAME}-${F_NAME:-ALL}" type="${CASE_TYPE}" level="${CASE_LEVEL}">
        <description>${EXEC_BASE_NAME}-${F_NAME:-ALL}</description>
        <step>${TESTS_INSTALL_DIR}/${EXEC_BASE_NAME} ${F_NAME}</step>
      </case>
END
}

generate_test_cases()
{
    EXECUTABLE="${1?}"
    CASE_TYPE="${2?}"
    CASE_LEVEL="${3?}"
    FLAGS="${4}"

    F_NAMES="`${EXECUTABLE} -functions`" || return 1

    if test_flag "${FLAGS}" "single-case"
    then
        fmt_test_case "${EXECUTABLE}" "" "${CASE_TYPE}" "${CASE_LEVEL}"
    else
        for f_name in ${F_NAMES}
        do
            f_name="${f_name/(*)/}"
            fmt_test_case "${EXECUTABLE}" "${f_name}" "${CASE_TYPE}" "${CASE_LEVEL}"
        done
    fi
}

generate_test_set()
{
    EXECUTABLE="${1?}"
    FEATURE="${2?}"
    CASE_TYPE="${3?}"
    CASE_LEVEL="${4?}"
    FLAGS="${5}"

    EXEC_BASE_NAME="`basename "${EXECUTABLE}"`"

    cat <<END
    <set name="${TESTS_INSTALL_DIR}/${EXEC_BASE_NAME}" feature="${FEATURE}">
      <pre_steps>
        <step>
          export SSO_LOGGING_LEVEL=2
          export SSO_LOGGING_OUTPUT="stdout"
          export SSO_STORAGE_PATH="/tmp"
          export SSO_DAEMON_TIMEOUT=5
          export SSO_IDENTITY_TIMEOUT=5
          export SSO_AUTHSESSION_TIMEOUT=5

          while ps -C signond &amp;>/dev/null; do pkill signond; sleep 1; done

          # Redirecting output to file prevents receiving SIGHUP which leads to
          # server restart and causes certain test cases to fail.
          signond 1>/tmp/tests-libsignond-signond.out 2>&amp;1 &amp;
`test_flag "${FLAGS}" "stop-ui" && echo "
          # Any signon UI must be stopped prior to executing this test set
          systemctl --user list-unit-files \\\\
            |awk '\\\$1 ~ /-signon-ui.service$/ { print \\\$1 }' \\\\
            |xargs systemctl --user stop
"`

          sleep 2
        </step>
      </pre_steps>
`generate_test_cases "${EXECUTABLE}" "${CASE_TYPE}" "${CASE_LEVEL}" "${FLAGS}"`
      <environments>
        <scratchbox>true</scratchbox>
        <hardware>true</hardware>
      </environments>
      <get>
        <file delete_after="true">/tmp/tests-libsignond-signond.out</file>
      </get>
    </set>
END
}

TESTS_INSTALL_DIR="${1?}"
HAVE_AEGIS="${2:-false}"

set -o errexit -o errtrace
trap "kill -QUIT $$" ERR
trap "exit 1" QUIT

cat <<END
<?xml version="1.0" encoding="ISO-8859-1"?>
<testdefinition version="1.0">

  <suite name="libsignon-qt-tests" domain="Accounts and SSO">
    <description>Signon Qt Client Library Tests</description>

`generate_test_set libsignon-qt-tests/libsignon-qt-tests \
    "SSO API" "Functional" "Component" "single-case,stop-ui"`

`${HAVE_AEGIS} || echo "    <!-- AEGIS not available on this platform"`
`generate_test_set libsignon-qt-tests/libsignon-qt-untrusted-tests \
    "SSO API" "Security" "Component" "single-case"`
`${HAVE_AEGIS} || echo "    -->"`

  </suite>


  <suite name="signon-passwordplugin-tests" domain="Accounts and SSO">

`generate_test_set passwordplugintest/signon-passwordplugin-tests \
    "Password" "Functional" "Component"`

  </suite>


  <suite name="signond" domain="Accounts and SSO">

`generate_test_set signond-tests/signon-tests \
    "SSO CORE" "FIXME" "FIXME" "single-case"`

  </suite>


  <suite name="signon-testpluginproxy" domain="Accounts and SSO">

`generate_test_set pluginproxytest/testpluginproxy \
    "FIXME" "FIXME" "FIXME" "single-case"`

  </suite>

</testdefinition>
END
