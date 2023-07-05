Linux Package dependencies (Ubuntu)

```apt install libsdl2-dev```

Building on Linux

```clang -o moria_at -I. `sdl2-config --cflags` `sdl2-config --libs` src/moria_at.c```

Running on Linux
```./moria_at```
