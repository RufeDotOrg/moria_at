#!/bin/bash

MAJOR=`cat major | wc -l`
MINOR=`cat minor | wc -l`
COUNT=$((`cat release | wc -l`+1))
VER=`printf "%04d.%04d.%04d" $((MAJOR&0x3ff)) $((MINOR&0x3ff)) $((COUNT&0x3ff))`
HASH=`git log -1 --format=%H | cut -c 1-15`
echo "Version $VER Git Hash $HASH"

while getopts "a" opt; do
  case ${opt} in
    a )
      SOURCE=AndroidManifest.xml
      TARGET=SDL2.24.2/build/org.rufe.moria/app/src/main/AndroidManifest.xml
      cp $SOURCE $TARGET
      sed -i "s/ZZZZ\>/$COUNT/g" $TARGET || exit 2
      sed -i "s/\<AbCdEfGhIjKlMnO\>/$VER.$HASH/g" $TARGET || exit 3
      exit 0
      ;;
  esac
done
shift $(($OPTIND - 1))

[ $# -eq 1 ] || exit 1

grep -q XXXX.YYYY.ZZZZ $1 || exit 1
sed -i "s/\<XXXX.YYYY.ZZZZ\>/$VER/g" $1 || exit 2
sed -i "s/\<AbCdEfGhIjKlMnO\>/$HASH/g" $1 || exit 3
echo "Modified binary $1"
