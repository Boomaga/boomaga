#!/bin/sh

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
