#!/bin/bash
NAME=cave
ffmpeg -y -framerate 1 -i cave_image%03d.png -vf 'scale=iw*5:ih*5,pad=720:405:(((ow - iw)/2 )):(( (oh - ih)/2 ))' $NAME.mp4
echo $? : $NAME.mp4
ffmpeg -y -i $NAME.mp4 $NAME.gif 
echo $? : $NAME.gif
