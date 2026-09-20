#!/bin/sh

set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
translations_dir="$repo_root/src/boomaga/translations"

if [ "$#" -gt 0 ]; then
    lupdate_bin=$1
elif [ -n "${LUPDATE:-}" ]; then
    lupdate_bin=$LUPDATE
elif command -v lupdate-qt6 >/dev/null 2>&1; then
    lupdate_bin=$(command -v lupdate-qt6)
elif [ -x /usr/lib/qt6/bin/lupdate ]; then
    lupdate_bin=/usr/lib/qt6/bin/lupdate
elif command -v lupdate >/dev/null 2>&1 && lupdate -version 2>&1 | grep -q 'version 6\.'; then
    lupdate_bin=$(command -v lupdate)
else
    echo "Qt 6 lupdate was not found. Set LUPDATE or pass its path as the first argument." >&2
    exit 1
fi

set -- "$translations_dir/src.boomaga.ts"
for ts_file in "$translations_dir"/boomaga_*.ts; do
    # Qt 6 does not recognize this legacy POSIX language tag. Weblate fills in
    # missing Uzbek units after importing the synchronized source strings.
    if [ "$(basename -- "$ts_file")" = "boomaga_uz@Latn.ts" ]; then
        continue
    fi
    set -- "$@" "$ts_file"
done

"$lupdate_bin" \
    -no-obsolete \
    -locations none \
    "$repo_root/src/boomaga" \
    -ts "$@"
