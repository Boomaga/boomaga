#!/usr/bin/env bash

set -euo pipefail

usage() {
    printf 'Usage: %s VERSION\n' "$(basename "$0")" >&2
    printf 'Examples: %s 3  |  %s 3.1.0  |  %s 3.1.0-beta1\n' "$(basename "$0")" "$(basename "$0")" "$(basename "$0")" >&2
    exit 2
}

[[ $# -eq 1 ]] || usage

version=$1
if [[ $version =~ ^([0-9]+)(\.([0-9]+))?(\.([0-9]+))?(-([[:alnum:].-]+))?$ ]]; then
    major=${BASH_REMATCH[1]}
    minor=${BASH_REMATCH[3]:-0}
    patch=${BASH_REMATCH[5]:-0}
    beta=${BASH_REMATCH[7]:-}
else
    printf 'Invalid version: %s\n' "$version" >&2
    usage
fi

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cmake_file="$repo_root/CMakeLists.txt"

[[ -f $cmake_file ]] || {
    printf 'Unable to find %s\n' "$cmake_file" >&2
    exit 1
}

if [[ -n $beta ]]; then
    beta_line="set(BETA_VERSION $beta)"
else
    beta_line="#set(BETA_VERSION beta5)"
fi

VERSION_MAJOR=$major VERSION_MINOR=$minor VERSION_PATCH=$patch \
VERSION_BETA_LINE="$beta_line" \
perl -0pi -e '
    s/^set\(MAJOR_VERSION\s+\d+\)$/set(MAJOR_VERSION $ENV{VERSION_MAJOR})/m;
    s/^set\(MINOR_VERSION\s+\d+\)$/set(MINOR_VERSION $ENV{VERSION_MINOR})/m;
    s/^set\(PATCH_VERSION\s+\d+\)$/set(PATCH_VERSION $ENV{VERSION_PATCH})/m;
    s/^#?set\(BETA_VERSION\s+[^)]+\)$/$ENV{VERSION_BETA_LINE}/m;
' "$cmake_file"

printf 'Updated Boomaga version to %s%s in %s\n' "$major.$minor.$patch" "${beta:+-$beta}" "$cmake_file"
