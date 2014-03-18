#!/bin/sh

set -e

echo -------------------------------------------
echo --               Git Hooks               --
echo -------------------------------------------

# Goto root.
pushd $(git rev-parse --show-toplevel || echo ".")

find . -path ./s/hooks/config/\* -exec chmod +x "{}" \;
find . -path ./s/hooks/config/\* -exec cp "{}" .git/hooks \;

popd

echo -------------------------------------------
echo --                 Done!                 --
echo -------------------------------------------
