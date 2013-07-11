#! /bin/sh

# If there's already an instance of signond running, kill it

set -e

# start a local signond

export SSO_LOGGING_LEVEL=2
export SSO_LOGGING_OUTPUT="stdout"
export SSO_STORAGE_PATH="/tmp"
export SSO_DAEMON_TIMEOUT=5
export SSO_IDENTITY_TIMEOUT=5
export SSO_AUTHSESSION_TIMEOUT=5
export PATH="${BUILDDIR}/src/remotepluginprocess:$PATH"
export LD_LIBRARY_PATH="${BUILDDIR}/lib/plugins/signon-plugins-common":"${BUILDDIR}/lib/signond/SignOn":"$LD_LIBRARY_PATH"

# If dbus-test-runner exists, use it to run the tests in a separate D-Bus
# session
if command -v dbus-test-runner > /dev/null ; then
    echo "Using dbus-test-runner"
    dbus-test-runner -m 180 -t ${SRCDIR}/tests/signond.sh \
        -t "$@" -f com.google.code.AccountsSSO.SingleSignOn
else
    echo "Using existing D-Bus session"
    pkill signond || true
    trap "pkill -9 signond" EXIT
    ${SRCDIR}/tests/signond.sh &
    sleep 2

    ${CLIENT_WRAPPER} $@
fi

