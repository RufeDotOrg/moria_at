#!/bin/bash
function exit_with_usage()
{
  printf "USAGE: $0 <platform>\n\n"
  printf "Supported platforms:\n"
  ls platform
  exit 1
}
[ $# -eq 1 ] || exit_with_usage
ln -sfn `realpath platform/$1` src/platform
readlink src/platform
