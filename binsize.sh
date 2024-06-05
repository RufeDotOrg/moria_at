#!/bin/bash
RANGE=818874ce1c79d259e78ffc97c295df22121b835e..aws/steam

printf "Check for uncommitted git changes... "
[ -z `git ls-files -m` ] || exit 1
echo OK

HASH_LIST=(`git log "$RANGE" --format=%H`)
echo "Hash List: ${#HASH_LIST[@]}"
[ "${#HASH_LIST[@]}" -gt 0 ] || exit 1

RESULT=()
SIZE=()
for h in ${HASH_LIST[@]}; do
  git reset --hard $h
  ,cc.sh src/moria_at.c &> /dev/null
  RESULT+=("$?")
  SIZE+=("`stat --format=%s bin/moria_at`")
done

# Output
for n in `seq 1 ${#HASH_LIST[@]}`; do
  idx=$((n-1))

  printf "%s: %d %d\n" ${HASH_LIST[idx]} ${RESULT[idx]} ${SIZE[idx]}
done

# Reset
git reset --hard "${HASH_LIST[0]}"
