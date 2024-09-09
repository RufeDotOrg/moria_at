
# Recommended Software
* gcc (11.4)
* clang (14)
* cosmocc (3.6.2)
* SDL2 (2.28.5)

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
clang -w -o moria_at -I. -lm `sdl2-config --cflags` `sdl2-config --libs` src/moria_at.c
```

## Running the Graphical Client
```
./moria_at
```

## Build Determinism
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
Gameplay side effects exist.
