#ifdef __FATCOSMOCC__
#include <math.h>
#endif

STATIC int lab2xyz(labptr, xyz) void* labptr;
float* xyz;
{
  int8_t* lab = labptr;
  float y = (lab[0] + 16) / 116.f;
  float x = lab[1] / 500.f + y;
  float z = y - lab[2] / 200.f;

  if (y >= 0.206893f)
    y = y * y * y;
  else
    y = (y - (16 / 116.f)) / 7.787;

  if (x >= 0.206893f)
    x = x * x * x;
  else
    x = (x - (16 / 116.f)) / 7.787;

  if (z >= 0.206893f)
    z = z * z * z;
  else
    z = (z - (16 / 116.f)) / 7.787;

  xyz[0] = 95.047f * x;
  xyz[1] = 100.f * y;
  xyz[2] = 108.883f * z;
  return 0;
}
// Returns true for valid Rgb range
STATIC int
xyz2rgb(xyz, result)
float* xyz;
void* result;
{
  float x = xyz[0] / 100.f, y = xyz[1] / 100.f, z = xyz[2] / 100.f;
  float r, g, b;

  r = (x * 3.2406) + (y * -1.5372) + (z * -0.4986);
  g = (x * -0.9689) + (y * 1.8758) + (z * 0.0415);
  b = (x * 0.0557) + (y * -0.2040) + (z * 1.0570);

  // assume sRGB
  r = r > 0.0031308 ? ((1.055 * powf(r, 1.0 / 2.4)) - 0.055) : (r * 12.92);

  g = g > 0.0031308 ? ((1.055 * powf(g, 1.0 / 2.4)) - 0.055) : (g * 12.92);

  b = b > 0.0031308 ? ((1.055 * powf(b, 1.0 / 2.4)) - 0.055) : (b * 12.92);

  int invalid = r < 0.f || r > 1.f || g < 0.f || g > 1.f || b < 0.f || b > 1.f;
  uint8_t* rgb = result;
  rgb[0] = CLAMP(r, 0.f, 1.f) * 255;
  rgb[1] = CLAMP(g, 0.f, 1.f) * 255;
  rgb[2] = CLAMP(b, 0.f, 1.f) * 255;
  return !invalid;
}
int
rgb_by_labr(labr)
{
  float xyz[3];
  int swrgb = -1;
  lab2xyz(vptr(&labr), xyz);
  if (xyz2rgb(xyz, &swrgb)) return swrgb;
  return 0;
}
#define COLOR 1
