// Rufe.org LLC 2022-2025: ISC License

// lum_white.c: 94
DATA int whiteD = 0xffededed;
// lum_color.c: LR 32, palette 7, odd indexes
DATA int color_mapD[] = {
    0xff1b0396, 0xff00539a, 0xff0299a1, 0xff109c04, 0xff8a9c02, 0xffa55907,
    0xff9e036d, 0xff62049b, 0xff5f65f2, 0xff509df2, 0xff5fe5f3, 0xff68f079,
    0xffdbf078, 0xfff19d6e, 0xffed5dbc, 0xffb06af4,
};
// lum: 7,14,21,30
DATA int lightingD[] = {
    0xff151515,
    0xff252525,
    0xff393939,
    0xff4f4f4f,
};
// lum: 30,37,45,52,60,67,75,82
// lum = 30 + it * 60.f / 8.f
DATA int greyscaleD[] = {0xff464646, 0xff575757, 0xff6a6a6a, 0xff7b7b7b,
                         0xff909090, 0xffa3a3a3, 0xffb8b8b8, 0xffcbcbcb};

STATIC int
rgba_by_palette(c)
{
  return color_mapD[c];
}
STATIC int
rgba_by_grey(c)
{
  return greyscaleD[c];
}
#define font_rgba rgba_by_palette

#define COLOR 1
