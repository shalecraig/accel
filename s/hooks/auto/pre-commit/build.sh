#!/bin/sh
set -e

pushd $(git rev-parse --show-toplevel || echo ".")

make clean run

popd
