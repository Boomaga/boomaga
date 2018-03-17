#!/bin/bash

SRC_COLOR="#656565"

ENABLE="#656565"
DISABLE="#a1a1a1"

RC_FILE="icons.qrc"

#ENABLE="#ff00ff"
#DISABLE="#00ff00"

function conv()
{
	local svgFile=$1
	#local pngFile=$2
	local size=$2
	local color=$3

	postfix=""
#	postfix="${size}x${size}"	
	[ "${color}" = "${DISABLE}" ] && postfix="_disable"

	pngFile=${svgFile/.svg/${postfix}.png}
	echo "$svgFile => $pngFile"

	sed -e "s/${SRC_COLOR}/${color}/g" ${svgFile} > icons/_tmp.svg
	mkdir -p icons/${size}
	inkscape -z -e icons/${size}/${pngFile} -w ${size} -h ${size} icons/_tmp.svg
	rm icons/_tmp.svg

	name=${svgFile/.svg/${postfix}}
	echo "        <file alias=\"${name}\">icons/${size}/${pngFile}</file>" >> ${RC_FILE}	
	echo ""
}

echo "<!-- Generated from icons2png.sh do not edit by hand. -->" > ${RC_FILE}
echo "<RCC>" >> ${RC_FILE}


for size in 16 22 32 48 64 128 256 512 ; do
	echo "    <qresource prefix=\"${size}\">" >> ${RC_FILE}	

	for color in "$ENABLE" "$DISABLE"; do 
		conv print.svg $size $color
		conv arrow-left.svg $size $color
		conv arrow-right.svg $size $color

		echo "" >> ${RC_FILE}
	done

#	conv track-cancel.svg $size ""
#	conv warning.svg $size ""

	echo "    </qresource>" >> ${RC_FILE}
	echo "" >> ${RC_FILE}
done

echo "</RCC>" >> ${RC_FILE}
