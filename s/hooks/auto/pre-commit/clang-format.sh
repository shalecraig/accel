#!/bin/sh
set -e

pushd $(git rev-parse --show-toplevel || echo ".")

git diff --name-only --cached | \
    grep -E '\.(h|c|cc|cpp)' | \
    xargs clang-format -i

popd
