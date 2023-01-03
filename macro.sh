#!/bin/bash
[ $# -eq 1 ] || exit 1
M=`printf "ARR_REUSE($1, 256);" | clang -E -xc -include src/game/common.h -include src/game/const.h -include src/game/type.h - | tail -1`
[ -n "$M" ] || exit 2
echo $M | clang-format
