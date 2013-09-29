#!/bin/bash

# BEGIN_COMMON_COPYRIGHT_HEADER
# (c)LGPL2+
#
#
# Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
# Authors:
#   Alexander Sokoloff <sokoloff.a@gmail.com>
#
# This program or library is free software; you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#
# END_COMMON_COPYRIGHT_HEADER

#ALL=''
#ONLY_ERRORS=1
COLOR='YES'

SEARCH="${SEARCH:-searchGit}"
DIR="${1:-../}"

if [ -n "$COLOR" ]; then
    RED='\E[0;31m'
    YELLOW='\E[0;33m'
    GREEN='\e[0;32m'
    NORM='\E[0;37;40m'
    RED_BG='\E[41m'
fi


#let 'invlid=0'
#let 'lgplCnt=0'
#let 'gplCnt=0'
#let 'unknownCnt=0'

searchAll()
{
    find ${DIR} -type f \( -name '*.h' -o -name '*.cpp' \)  2>/dev/null
}

searchGit()
{
    git ls-files -- "${DIR}" | grep -E '(*\.cpp|*\.h)$'
}

# License compatibility: BSD 3-Clause; LGPL v2.1 or later

$SEARCH | while read file; do
    license=`head -n 5 "$file"| grep '(c)' | sed -e 's/*//'`;# | sed -e 's/\([()]\)/\\1/g'`;

    case "$license" in
        *LGPL2+*|*DWTFYW*|*BSD*)
            [ -z "$ALL" ] && continue
            color=$GREEN
            ;;

        *LGPL3+*)
            color=$GREEN
            ;;

        *LGPL2*|*LGPL3*)
            [ -n "$ONLY_ERRORS" ] && continue
            color=$YELLOW
            ;;

        *GPL2*|*GPL3*)
            [ -n "$ONLY_ERRORS" ] && continue
            color=$RED
            ;;

        *)
            color=$RED_BG
            [ -z "$license" ] && license='  Not set'
            ;;
    esac

    let "div = 20 - ${#license}"
    printf "${color}%-20s %s${NORM}\n"  "${license}" "$file"
done
#echo
#echo "LGPL:    $lgplCnt"
#echo "GPL:     $gplCnt"
#echo "Unknown: $unknownCnt"
