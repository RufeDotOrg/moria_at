#!/bin/bash
set_version() {
MAJOR=`cat major | wc -l`
MINOR=`cat minor | wc -l`
BUILD=`tail -1 release | cut -c 1-15`
COUNT=$((`cat release | wc -l`+1))
VER=`printf "%04d.%04d.%04d" $((MAJOR&0x3ff)) $((MINOR&0x3ff)) $((COUNT&0x3ff))`
HASH=`git log -1 --format=%H | cut -c 1-15`
echo "Version $VER Git Hash $HASH"
}

while getopts "ac:r" opt; do
  case ${opt} in
    a ) # add new version
      printf "Check for uncommitted git changes... "
      [ -z `git ls-files -m` ] || exit 1
      echo OK

      command git log -1 --format=%H >> release
      set_version
      SOURCE=AndroidManifest.xml
      TARGET=SDL2.24.2/build/org.rufe.moria/app/src/main/AndroidManifest.xml
      cp $SOURCE $TARGET
      sed -i "s/ZZZZ\>/$COUNT/g" $TARGET || exit 2
      sed -i "s/\<AbCdEfGhIjKlMnO\>/$VER.$HASH/g" $TARGET || exit 3
      exit 0
      ;;
    c ) # change list
      [ -z `git ls-files -m` ] || echo "***DIRTY REPOSITORY***"
      tail -$((OPTARG+1)) release | head -1 | xargs -I{} git log --format=%s {}..master
      exit 0
      ;;
    r ) # ready
      printf "Check build is latest... "
      [ `tail -1 release` == `git log -1 --format=%H` ] || exit 1
      echo OK
      exit 0
  esac
done
shift $(($OPTIND - 1))

set_version
[ "$BUILD" == "$HASH" ] || echo "release may be added with -a"
[ $# -eq 1 ] || exit 1

[ "$BUILD" == "$HASH" ] || exit 1
grep -q XXXX.YYYY.ZZZZ $1 || exit 1
sed -i "s/\<XXXX.YYYY.ZZZZ\>/$VER/g" $1 || exit 2
sed -i "s/\<AbCdEfGhIjKlMnO\>/$HASH/g" $1 || exit 3
echo "Modified binary $1"
