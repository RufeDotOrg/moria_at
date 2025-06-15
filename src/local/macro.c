#define FDECL(f, p) f p
#define MSG(x, ...)                                   \
  {                                                   \
    int len = snprintf(AP(vtypeD), x, ##__VA_ARGS__); \
    msg_game(vtypeD, len, msg_widthD);                \
  }

#define CLOBBER_MSG(x, ...)                      \
  ({                                             \
    char* msg = AS(msg_cqD, msg_writeD);         \
    int len = STRLEN_MSG + 1;                    \
    snprintf(msg, len, x, ##__VA_ARGS__);        \
    AS(msglen_cqD, msg_writeD) = STRLEN_MSG + 1; \
    draw(DRAW_WAIT);                             \
  })

#define BufMsg(name, text, ...)                                     \
  {                                                                 \
    int used = name##_usedD[line];                                  \
    int avail = AL(name##D[0]) - used;                              \
    int r;                                                          \
    r = snprintf(name##D[line] + used, avail, text, ##__VA_ARGS__); \
    if (r > 0 && r <= avail) name##_usedD[line++] = used + r;       \
  }
#define BufFixed(name, text)                  \
  {                                           \
    struct bufS b = {AP(text)};               \
    memcpy(name##D[line], b.mem, b.mem_size); \
    name##_usedD[line] = b.mem_size;          \
    line += 1;                                \
  }
#define BufLineAppend(name, line, text, ...)                             \
  {                                                                      \
    int used, append;                                                    \
    used = name##_usedD[line];                                           \
    append = snprintf(name##D[line] + used, AL(name##D[0]) - used, text, \
                      ##__VA_ARGS__);                                    \
    if (append > 0) name##_usedD[line] += append;                        \
  }

#define BufPad(name, line, len)            \
  for (int it = 0; it < line; ++it) {      \
    while (name##_usedD[it] < len) {       \
      name##D[it][name##_usedD[it]] = ' '; \
      name##_usedD[it] += 1;               \
    }                                      \
  }

#define msg_print(x) msg_write(S2(x), msg_widthD)
#define see_print(x) \
  if (maD[MA_BLIND] == 0) msg_print(x)

#define RNG_M 2147483647LL
#define RNG_A 16807LL
#define RNG_Q 127773LL
#define RNG_R 2836LL

#define ADJ4(y, x, body)      \
  {                           \
    struct caveS* c_ptr;      \
    c_ptr = &caveD[y - 1][x]; \
    body;                     \
    c_ptr = &caveD[y + 1][x]; \
    body;                     \
    c_ptr = &caveD[y][x - 1]; \
    body;                     \
    c_ptr = &caveD[y][x + 1]; \
    body;                     \
  }

#define TOMB(x, ...)                                     \
  {                                                      \
    int len = snprintf(tmp, AL(tmp), x, ##__VA_ARGS__);  \
    memcpy(&screenD[line][xcenter - len / 2], tmp, len); \
    line += 1;                                           \
  }

