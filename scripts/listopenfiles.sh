#!/bin/sh
PID=`ps -A | grep boomaga | cut -d" " -f2` 
echo $PID
ls -l /proc/${PID}/fd | grep -v 'pipe:' | grep -v 'socket:' | grep -v 'inode:'

