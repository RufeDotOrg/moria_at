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

  y = y * y * y;
  x = x * x * x;
  z = z * z * z;

  xyz[0] = .95047f * x;
  xyz[1] = y;  // 100.f * y;
  xyz[2] = 1.08883f * z;
  return 0;
}

STATIC int
xyz2rgb(xyz, result)
float* xyz;
void* result;
{
  float x = xyz[0];
  float y = xyz[1];
  float z = xyz[2];

  // sRGB http://www.brucelindbloom.com/Eqn_RGB_XYZ_Matrix.html
  float r = (x * 3.2406) + (y * -1.5372) + (z * -0.4986);
  float g = (x * -0.9689) + (y * 1.8758) + (z * 0.0415);
  float b = (x * 0.0557) + (y * -0.2040) + (z * 1.0570);
#define ZVAL 0.00084f
  int invalid = (b < ZVAL) || (g < ZVAL) || (r < ZVAL);

  if (!invalid) {
    r = ((1.055 * powf(r, 1.0 / 2.4)) - 0.055);
    g = ((1.055 * powf(g, 1.0 / 2.4)) - 0.055);
    b = ((1.055 * powf(b, 1.0 / 2.4)) - 0.055);

    invalid = r > 1.f || g > 1.f || b > 1.f;

    if (!invalid) {
      bptr(result)[0] = r * 255;
      bptr(result)[1] = g * 255;
      bptr(result)[2] = b * 255;
    }
  }

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
