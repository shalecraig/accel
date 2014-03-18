#!/bin/sh
set -e
pushd $(git rev-parse --show-toplevel || echo ".")
clang-format -i src/*.{c,h} test/*.{cc,h} sample/**/*.{c,h,cc,cpp}
popd
