#!/bin/bash
nm $1 | grep -i ' b ' | cut -d ' ' -f 3 | xargs -I {} grep -rHn " \<{}\>" src/game/*.h | grep -v BUF | sort
