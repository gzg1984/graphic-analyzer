#!/bin/sh
# use xwininfo get current window ID
windowid=`xwininfo -root|grep "Window id"|sed -e "s/.*Window id:\(.*\)(the root.*/\1/g"`

xterm -into $windowid
