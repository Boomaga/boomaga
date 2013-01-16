#!/bin/sh

OPTIONS="-no-obsolete"
lupdate ${OPTIONS} ../*.h ../*.cpp ../*.ui ../*/*.h ../*/*.cpp ../*/*.ui -ts booklet_ru.ts
