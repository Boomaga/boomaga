#!/bin/sh
PID=`ps -A | grep boomaga | sed -e 's/^[ \t]*//' | cut -d" " -f1`
echo $PID
ls -l /proc/${PID}/fd | grep -v 'pipe:' | grep -v 'socket:' | grep -v 'inode:'

