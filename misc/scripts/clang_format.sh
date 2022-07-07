#!/usr/bin/env bash

# This script runs clang-format and fixes copyright headers on all relevant files in the repo.
# This is the primary script responsible for fixing style violations.

set -uo pipefail

# Loops through all code files tracked by Git.
git ls-files -- '*.c' '*.h' '*.cpp' '*.hpp' ':!:.git/*' ':!:deps/*' |
while read -r f; do
    # Run clang-format.
    clang-format --Wno-error=unknown -i "$f"

    # Fix copyright headers.
    python misc/scripts/copyright_headers.py "$f"
done

diff=$(git diff --color)

# If no diff has been generated all is OK, clean up, and exit.
if [ -z "$diff" ] ; then
    printf "Files in this commit comply with the clang-format style rules.\n"
    exit 0
fi

# A diff has been created, notify the user, clean up, and exit.
printf "\n*** The following changes have been made to comply with the formatting rules:\n\n"
echo "$diff"
printf "\n*** Please fix your commit(s) with 'git commit --amend' or 'git rebase -i <hash>'\n"
exit 1
