#! /bin/sh

set -e

BUILDDIR="${BUILDDIR:=..}"
SIGNOND="${BUILDDIR}/src/signond/signond"

echo "Starting signond from $SIGNOND"

$WRAPPER $SIGNOND

