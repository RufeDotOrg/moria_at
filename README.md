
# Recommended software
* gcc (11.4)
* clang (14)
* cosmocc (4.0.2)
* SDL2 (2.28.5)

# Build targets
1) command line terminal
2) graphical client using SDL2
3) portable deterministic graphical client using SDL2 + cosmocc
4) mobile devices (Android & iOS)

# Building for command line terminal (no dependencies)
```
rm src/platform
ln -s `realpath platform/linux` src/platform
clang -Wno-incompatible-function-pointer-types -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type -o moria_at -I. src/moria_at.c
```

## Command line wizard mode (character editor) Mac or Linux
```
rm src/platform
ln -s `realpath platform/linux` src/platform
clang -Wno-incompatible-function-pointer-types -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type -o wizard -I. src/wizard.c
```

# Graphical Client for Linux

## Install SDL2 package dependencies (Ubuntu):
```
apt install libsdl2-dev
```

## Compiling the graphical client
```
rm src/platform
ln -s `realpath platform/sdl2` src/platform
clang -Wno-incompatible-function-pointer-types -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type -o moria_at -I. -lm `sdl2-config --cflags` `sdl2-config --libs` src/moria_at.c
```

## Running the graphical client
```
./moria_at
```

# Portable deterministic graphical client
Use a shell variable for the absolute path of SDL2 ($SDLPATH)
Use a shell variable for the absolute path of cosmocc ($COSMOCC)
```
ln -sf $SDLPATH sdl2
LC_ALL=C SOURCE_DATE_DEPOCH=0 $COSMOCC src/moria_at.c -O1 -g1 -I. -fno-omit-frame-pointer -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type -Isdl2/include -DNDEBUG -DRELEASE -U__DATE__ -U__TIME__ -fdebug-prefix-map=$PWD=. -fno-math-errno -ffp-contract=fast -freciprocal-math -fno-trapping-math -lm -o bin/moria_at.exe
```

## Version stamping
Fixed length strings, assuming shell variables $VERSION, $HASH:
```
sed -i "s/\<XXXX.YYYY.ZZZZ\>/$VERSION/g" bin/moria_at.exe
sed -i "s/\<AbCdEfGhIjKlMnO\>/$HASH/g" bin/moria_at.exe
```
Gameplay side effects exist based on the client version.

# Mobile device builds

Follow platform instructions in submodule/sdl2/docs
