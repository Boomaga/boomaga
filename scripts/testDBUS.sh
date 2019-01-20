#!/bin/sh

FILE=$1

if [ "$FILE" = "" ]; then
	echo "Missing file operand"
	echo ""
	echo "Usage:  testDBUS.sh PDF_FILE"
	exit 1
fi

FILE=$(readlink -e "${FILE}")

###########################
# D-Bus arguments
#
# string file
# string title
# string options
# uint   count
dbus-send \
	--session \
	--type=method_call \
	--print-reply \
	--dest=org.boomaga /boomaga org.boomaga.add \
	string:"${FILE}" \
	string:'title' \
	string:'options' \
	uint32:1

