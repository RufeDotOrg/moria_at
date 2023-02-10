#!/bin/bash
[ -d bin/ ] || mkdir bin
readlink src/platform || ./init_platform.sh linux
[ -z "$CC" ] && CC=clang

SYMLEVEL=1
[ -n "$SYM" ] && SYMLEVEL=$SYM
OPTLEVEL=1
[ -n "$OPT" ] && OPTLEVEL=$OPT
if [ -n "$ASAN" ]; then
  OPTLEVEL=0
  SYMLEVEL=3
  DEV_FLAGS+=" -fsanitize=address "
fi

hash $CC || exit 1
[ -z "$CFLAGS" ] && CFLAGS="-O${OPTLEVEL} -g${SYMLEVEL} -I. -fno-omit-frame-pointer "

EXT=.${1##*.}
OUTFILE=`basename $1 $EXT`

case $CC in
  gcc )
    CFLAGS+=" -fmax-errors=3 "
    ;;
  clang )
    CFLAGS+=" -ferror-limit=3 "
    ;;
esac
case $EXT in
  .c )
    CFLAGS+=" -Wno-implicit-function-declaration "
    ;;
  .cc ) ;&
  .cpp )
    CFLAGS+=" -fno-asynchronous-unwind-tables -fno-rtti -fno-exceptions -std=c++17 "
    CFLAGS+=" -ffp-contract=off -freciprocal-math -fno-math-errno "

    LFLAGS+=" -lstdc++ "
    ;;
esac
CFLAGS+=" $DEV_FLAGS "

TDIR=`dirname $1`
[ -f ccext ] && source ccext
[ -f $TDIR/ccext ] && source $TDIR/ccext

echo $CC $1 $CFLAGS $LFLAGS -o bin/$OUTFILE
time $CC $1 $CFLAGS $LFLAGS -o bin/$OUTFILE || exit 1

if [ -n "$OD" ]; then
  objdump -C -Mintel -d bin/$OUTFILE > fullobj_$OUTFILE
fi

if [ -n "$VER" ]; then
  ./version.sh bin/$OUTFILE || exit 1
fi

if [ $# -gt 1 ]; then
  shift
  bin/$OUTFILE $@
fi
