#!/bin/bash
function exit_with_usage()
{
  echo "web.sh <source_folder> <dest_folder>"
  exit 1
}
[ $# -eq 2 ] || exit_with_usage

echo "updating $2"
mkdir -p $2

FILE_LIST=`find $1 -type f,l`
for file in ${FILE_LIST[@]}; do
  BASE=`basename $file`
  bin/tohtml $file > $2/$BASE.html || exit 2
done

