#!/bin/bash

PNG_NAME=boomaga
SVG_FILE=boomaga.svg


inkscape -z -e ${PNG_NAME}-16x16.png   -w  16 -h  16 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-32x32.png   -w  32 -h  32 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-48x48.png   -w  48 -h  48 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-64x64.png   -w  64 -h  64 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-128x128.png -w 128 -h 128 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-256x256.png -w 256 -h 256 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}-512x512.png -w 512 -h 512 ${SVG_FILE}

mkdir -p ${PNG_NAME}.iconset
inkscape -z -e ${PNG_NAME}.iconset/icon_512x512.png -w 512 -h 512 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_256x256.png -w 256 -h 256 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_128x128.png -w 128 -h 128 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_32x32.png   -w  32 -h  32 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_16x16.png   -w  16 -h  16 ${SVG_FILE}

inkscape -z -e ${PNG_NAME}.iconset/icon_512x512@2x.png -w 1024 -h 1024 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_256x256@2x.png -w  512 -h  512 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_128x128@2x.png -w  256 -h  256 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_32x32@2x.png   -w   64 -h   64 ${SVG_FILE}
inkscape -z -e ${PNG_NAME}.iconset/icon_16x16@2x.png   -w   32 -h   32 ${SVG_FILE}

