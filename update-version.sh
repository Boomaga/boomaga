#!/usr/bin/env bash

set -euo pipefail

usage() {
    printf 'Usage: %s VERSION MESSAGE\n' "$(basename "$0")" >&2
    printf 'Example: %s 3.1.0 "Initial Debian packaging"\n' "$(basename "$0")" >&2
    exit 2
}

[[ $# -eq 2 ]] || usage

version=$1
message=$2
[[ -n $message ]] || usage
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
changelog_file="$repo_root/debian/changelog"

for file in "$cmake_file" "$changelog_file"; do
    [[ -f $file ]] || {
        printf 'Unable to find %s\n' "$file" >&2
        exit 1
    }
done

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

distribution=$(sed -n '1s/^boomaga ([^)]*) \([^;]*\);.*/\1/p' "$changelog_file")
maintainer=$(sed -n 's/^ -- //p' "$changelog_file" | head -n 1 | sed 's/  [A-Z][a-z][a-z], .*//')
[[ -n $distribution && -n $maintainer ]] || {
    printf 'Unable to determine Debian changelog metadata from %s\n' "$changelog_file" >&2
    exit 1
}

temporary_changelog=$(mktemp "${changelog_file}.XXXXXX")
trap 'rm -f "$temporary_changelog"' EXIT
{
    printf 'boomaga (%s-1) %s; urgency=medium\n\n' "$version" "$distribution"
    printf '  * %s\n\n' "$message"
    printf ' -- %s  %s\n\n' "$maintainer" "$(date -R)"
    cat "$changelog_file"
} > "$temporary_changelog"
mv "$temporary_changelog" "$changelog_file"

printf 'Updated Boomaga version to %s%s in %s\n' "$major.$minor.$patch" "${beta:+-$beta}" "$cmake_file"
printf 'Added Debian changelog entry for %s in %s\n' "$version" "$changelog_file"
