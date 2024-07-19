
// int[4]
typedef struct rectS {
  int x;
  int y;
  int w;
  int h;
} rect_t;
// int[2]
typedef struct pointS {
  int x;
  int y;
} point_t;
typedef struct pointS vec2;
static int
point_in_rect(void* in_point, void* in_rect)
{
  point_t* p = in_point;
  rect_t* r = in_rect;
  uint32_t xdelta = p->x - r->x;
  uint32_t ydelta = p->y - r->y;

  return (xdelta < r->w && ydelta < r->h);
}
