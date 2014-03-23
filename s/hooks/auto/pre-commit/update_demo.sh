#!/bin/sh
set -e

pushd $(git rev-parse --show-toplevel || echo ".")

cp src/* sample/simple-accelerometer/src/

popd
