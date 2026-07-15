#!/bin/sh
mv "$1" "`echo "$1" | sed -e 's/\/\([^\/]*\.gcda\)$/\/.tmp_\1/'`"
