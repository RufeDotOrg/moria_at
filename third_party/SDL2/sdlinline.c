__attribute__((always_inline)) static __inline__ void SDL_memset4(void *dst, Uint32 val, size_t dwords)
{
    size_t _n = (dwords + 3) / 4;
    Uint32 *_p = ((Uint32 *)(dst));
    Uint32 _val = (val);
    if (dwords == 0) {
        return;
    }
    switch (dwords % 4) {
        case 0: do { *_p++ = _val; __attribute__((__fallthrough__));
        case 3: *_p++ = _val; __attribute__((__fallthrough__));
        case 2: *_p++ = _val; __attribute__((__fallthrough__));
        case 1: *_p++ = _val;
        } while ( --_n );
    }
}
__attribute__((always_inline)) static __inline__ void *SDL_memcpy4( void *dst, const void *src, size_t dwords)
{
    return SDL_memcpy(dst, src, dwords * 4);
}
__attribute__((always_inline)) static __inline__ int SDL_size_mul_overflow (size_t a,
                                            size_t b,
                                            size_t *ret)
{
    if (a != 0 && b > ((size_t) -1) / a) {
        return -1;
    }
    *ret = a * b;
    return 0;
}
__attribute__((always_inline)) static __inline__ int _SDL_size_mul_overflow_builtin (size_t a,
                                                     size_t b,
                                                     size_t *ret)
{
    return __builtin_mul_overflow(a, b, ret) == 0 ? 0 : -1;
}
__attribute__((always_inline)) static __inline__ int SDL_size_add_overflow (size_t a,
                                            size_t b,
                                            size_t *ret)
{
    if (b > ((size_t) -1) - a) {
        return -1;
    }
    *ret = a + b;
    return 0;
}
__attribute__((always_inline)) static __inline__ int _SDL_size_add_overflow_builtin (size_t a,
                                                     size_t b,
                                                     size_t *ret)
{
    return __builtin_add_overflow(a, b, ret) == 0 ? 0 : -1;
}
__attribute__((always_inline)) static __inline__ float
SDL_SwapFloat(float x)
{
    union {
        float f;
        Uint32 ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = __builtin_bswap32(swapper.ui32);
    return swapper.f;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
    return ( (p->x >= r->x) && (p->x < (r->x + r->w)) &&
             (p->y >= r->y) && (p->y < (r->y + r->h)) ) ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_RectEmpty(const SDL_Rect *r)
{
    return ((!r) || (r->w <= 0) || (r->h <= 0)) ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_RectEquals(const SDL_Rect *a, const SDL_Rect *b)
{
    return (a && b && (a->x == b->x) && (a->y == b->y) &&
            (a->w == b->w) && (a->h == b->h)) ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_PointInFRect(const SDL_FPoint *p, const SDL_FRect *r)
{
    return ( (p->x >= r->x) && (p->x < (r->x + r->w)) &&
             (p->y >= r->y) && (p->y < (r->y + r->h)) ) ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_FRectEmpty(const SDL_FRect *r)
{
    return ((!r) || (r->w <= 0.0f) || (r->h <= 0.0f)) ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_FRectEqualsEpsilon(const SDL_FRect *a, const SDL_FRect *b, const float epsilon)
{
    return (a && b && ((a == b) ||
            ((SDL_fabsf(a->x - b->x) <= epsilon) &&
            (SDL_fabsf(a->y - b->y) <= epsilon) &&
            (SDL_fabsf(a->w - b->w) <= epsilon) &&
            (SDL_fabsf(a->h - b->h) <= epsilon))))
            ? SDL_TRUE : SDL_FALSE;
}
__attribute__((always_inline)) static __inline__ SDL_bool SDL_FRectEquals(const SDL_FRect *a, const SDL_FRect *b)
{
    return SDL_FRectEqualsEpsilon(a, b, 1.1920928955078125e-07F);
}
