#!/bin/bash

MAJOR=`cat major | wc -l`
MINOR=`cat minor | wc -l`
COUNT=`cat release | wc -l`
VER=`printf "%04d.%04d.%04d" $((MAJOR&0x3ff)) $((MINOR&0x3ff)) $((COUNT&0x3ff))`
HASH=`git log -1 --format=%H | cut -c 1-15`
echo "Version $VER Git Hash $HASH"

[ $# -eq 1 ] || exit 1
sed -i "s/\<XXXX.YYYY.ZZZZ\>/$VER/g" $1 || exit 2
sed -i "s/\<AbCdEfGhIjKlMnO\>/$HASH/g" $1 || exit 3
echo Modified executable $1
