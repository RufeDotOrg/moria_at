
# Recommended C compilers
* gcc
* clang
* cosmocc

# Building for command line terminal (no dependencies)
```
rm src/platform
ln -s `realpath platform/linux` src/platform
clang -w -o moria_at -I. src/moria_at.c
```

# Building Wizard Mode (character editor) Mac or Linux
```
rm src/platform
ln -s `realpath platform/linux` src/platform
clang -w -o wizard -I. src/wizard.c
```

# Graphical Client for Linux

## Install SDL2 Package dependencies (Ubuntu):
```
apt install libsdl2-dev
```

## Compiling the Graphical Client
```
rm src/platform
ln -s `realpath platform/sdl2` src/platform
clang -w -o moria_at -I. `sdl2-config --cflags` `sdl2-config --libs` src/moria_at.c
```

## Running the Graphical Client
```
./moria_at
```
