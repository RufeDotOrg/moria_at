

STATIC int
apclear(dst, dlen)
char* dst;
{
  memset(dst, 0, dlen);
}
STATIC int
apspace(dst, dlen)
char* dst;
{
  memset(dst, 0x20202020, dlen);
}

STATIC int
apcopy(dst, dlen, src, slen)
char* dst;
char* src;
{
  if (slen <= dlen) memcpy(dst, src, slen);
  return (slen <= dlen) * slen;
}

STATIC int
apcat(dst, dlen, src)
char* dst;
char* src;
{
  char c;
  char* start = dst;
  char* iter = dst;
  while (dlen > 0 && *iter) {
    ++iter;
    --dlen;
  }

  while ((c = *src++)) {
    if (dlen) {
      *iter++ = c;
      --dlen;
    }
  }
  if (dlen) *iter = 0;
  return iter - start;
}

STATIC int
apcati(dst, dlen, num)
char* dst;
{
  char* iter = dst;
  for (; *iter; ++iter) --dlen;
  if (num < 0) num = -num;

  // digits from right to left
  char* start = iter;
  for (int it = 0; it < dlen; ++it) {
    if (num == 0) break;
    *iter++ = '0' + (num % 10);
    num /= 10;
  }

  if (num == 0) {
    char* last = iter - 1;
    while (start < last) {
      char temp = *start;
      *start++ = *last;
      *last-- = temp;
    }
  } else {
    iter = start;
  }

  *iter = '\0';
  return iter - dst;
}
