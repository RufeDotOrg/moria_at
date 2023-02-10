#!/bin/bash
nm $1 | grep -i ' b ' | cut -d ' ' -f 3 | xargs -I {} grep -Hn {} src/game/*.h
