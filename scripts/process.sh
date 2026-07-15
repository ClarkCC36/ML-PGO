#!/bin/bash

if test -z "$1" || test  ! -f "$1" ; then
echo $0 profile.tar.gz
exit 1
fi

rm -fr sys
tar xzf "$1"
find sys -name '*.gcda' > list.txt
./calcsum list.txt
rm -f list.txt
find sys -name '*.gcda' -exec ./rename.sh {} \;
