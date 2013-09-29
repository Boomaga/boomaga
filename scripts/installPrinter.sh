#!/bin/sh

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

SCHEME="boomaga"
URI="${SCHEME}:/"
NAME="Boomaga_Printer"
PPD="lsb/usr/${SCHEME}/${SCHEME}.ppd"

while [ $# -gt 0 ]; do
  case $1 in
    -f|--force)
      FORCE=yes
      shift
      ;;  

  *)
      shift
      ;;  

  esac
done  


if [ -z "${FORCE}" ]
then
  if [ -n "$(LC_ALL=C lpstat -h localhost -v 2>/dev/null | grep "$URI")" ] 
  then
    echo "Looks like ${URI} already installed. Use --force option." >&2
    exit 1
  fi
fi

printer=${NAME}
while $(LC_ALL=C lpstat -h localhost -v 2>/dev/null | cut -d ':' -f 1 | cut -d ' ' -f 3 | grep -q "^${printer}"\$)
do
  number=$(($number + 1))
  printer="${NAME}-${number}"
done

pageSize="$(LC_ALL=C paperconf 2>/dev/null)" || size=a4


lpadmin -h localhost -p "${printer}" -v ${URI} -E -m ${PPD} -o printer-is-shared=no -o PageSize=${pageSize}


if [ -z "$(LC_ALL=C lpstat -h localhost -d 2>/dev/null | grep 'system default destination:')" ]
then
  lpadmin -h localhost -d "${printer}"
fi

echo "Printer ${printer} has been installed successfully."
exit 0
