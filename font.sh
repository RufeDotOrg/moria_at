#!/bin/bash
zip font.zip font
xxd -seek 62 -i font.zip > src/platform/font_zip.c
rm font.zip
