
// Third party
#include "third_party/zlib/puff.c"

enum { CHUNK = 32 * 1024 };
DATA uint8_t puff_buffD[2 * CHUNK];

int
puffex_stream_len(callback, stream, len)
fn callback;
void* stream;
{
  uint64_t source_len = len;
  uint64_t dest_len = AL(puff_buffD);
  return puff_stream_decode(puff_buffD, &dest_len, stream, &source_len,
                            callback);
}

int
puff_stream_decode(
    unsigned char* dest,         /* pointer to destination pointer */
    unsigned long* destlen,      /* amount of output space */
    const unsigned char* source, /* pointer to source data pointer */
    unsigned long* sourcelen, fn callback)
{
  struct state s; /* input/output state */
  int last, type; /* block information */
  int err;        /* return value */
  int trimlen;

  /* initialize output state */
  s.out = dest;
  s.outlen = *destlen; /* ignored if dest is 0 */
  s.outcnt = 0;
  trimlen = *destlen / 2;

  /* initialize input state */
  s.in = source;
  s.inlen = *sourcelen;
  s.incnt = 0;
  s.bitbuf = 0;
  s.bitcnt = 0;

  /* return if bits() or decode() tries to read past available input */
  if (setjmp(s.env) != 0) /* if came back here via longjmp() */
    err = 2;              /* then skip do-loop, return error */
  else {
    /* process blocks until last block or error */
    do {
      last = bits(&s, 1); /* one if last block */
      type = bits(&s, 2); /* block type 0..3 */
      err = type == 0 ? stored(&s)
                      : (type == 1 ? fixed(&s)
                                   : (type == 2 ? dynamic(&s)
                                                : -1)); /* type == 3, invalid */
      if (s.outcnt > trimlen) {
        // process & trim
        callback(dest, trimlen);
        memmove(dest, &dest[trimlen], s.outcnt - trimlen);
        s.outcnt -= trimlen;
      }
      if (err != 0) {
        break; /* return with error */
      }
    } while (!last);
    if (s.outcnt > 0) callback(dest, s.outcnt);
  }

  /* update the lengths and return */
  if (err <= 0) {
    *destlen = s.outcnt;
    *sourcelen = s.incnt;
  }
  return err;
}

#define PUFF_STREAM 1
