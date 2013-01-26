#!/bin/sh

LUPDATE_OPTIONS="-no-obsolete"
cd ../gui/translations && pwd && lupdate ${OPTIONS} ../*.h ../*.cpp ../*.ui ../*/*.h ../*/*.cpp ../*/*.ui -ts src.boomaga.ts
tx push --source
