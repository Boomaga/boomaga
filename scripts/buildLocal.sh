#!/bin/sh
BUILD_DIR=../build
INSTALL_DIR=$(readlink -e "${BUILD_DIR}")/OUT

echo "BUILD DIR:   ${BUILD_DIR}"
echo "INSTALL DIR: ${INSTALL_DIR}"
set -e

mkdir -p ${BUILD_DIR} 

cd ${BUILD_DIR}

cmake \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/usr \
	-DCUPS_BACKEND_DIR=${INSTALL_DIR}/usr/lib/cups/backend \
	-DCUPS_FILTER_DIR=${INSTALL_DIR}/usr/lib/cups/filter \
	-DCUPS_PPD_DIR=${INSTALL_DIR}/usr/share/ppd/boomaga \
	-DDBUS_MACINE_ID_DIR=${INSTALL_DIR}/var/lib/dbus \
	..

make

make install