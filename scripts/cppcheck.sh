#!/bin/sh
cppcheck --force -q  --enable=performance,portability,warning,style --library=qt ..
