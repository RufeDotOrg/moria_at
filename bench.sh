#!/bin/bash
FILE=timing7b
sed -i "s/'//g" $FILE
LIST=(`cat $FILE`)
p=0
idx=1
for l in ${LIST[@]}; do
  DELTA=$((l-p))
  if [ $DELTA -gt 10000 ]; then
    echo $idx: $l $DELTA
  fi
  p=$l
  idx=$((idx+1))
done
