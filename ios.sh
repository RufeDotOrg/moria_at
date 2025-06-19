#!/bin/bash
pushd ss
  DIR_LIST=`ls */ -d`
  for d in ${DIR_LIST[@]}; do
    pushd $d
      ../../bin/screenshot portrait.nmg landscape.nmg
    popd
  done
popd
