#!/bin/sh

FILE=$1

if [ "$FILE" = "" ]; then
	echo "Missing file operand"
	echo ""
	echo "Usage:  testBackend.sh PDF_FILE"
	exit 1
fi

FILE=$(readlink -e "${FILE}")


boomagabackend=""

dir=$(pwd)
while [[ "$dir" != "/" ]]; do
	if [ -d "${dir}/.git" ]; then 
		boomagabackend=$(find "$dir" -name "boomagabackend")
		break
	fi

	dir=$(dirname $dir)
	sleep 1
done


if [ "$boomagabackend" == "" ]; then
	boomagabackend=$(find "/usr/local/lib" "/usr/lib" -name "boomagabackend" 2>/dev/null)
fi

if [ "$boomagabackend" == "" ]; then
	"echo boomagabackend not found"
fi


echo "################"
echo "# program - $boomagabackend"
echo "# file    - $FILE"
echo "#"


#               <jobId> <title> <count> <options> <user>
cat "${FILE}" | $boomagabackend 123 	"title"	1 		""		  $USER 	