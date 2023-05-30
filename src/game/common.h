// stdint
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef int (*fn)();

#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#define UINT32_MAX __UINT32_MAX__
#define UINT16_MAX __UINT16_MAX__
#define UINT8_MAX __UINT8_MAX__
#define INT64_MAX __INT64_MAX__
#define INT32_MAX __INT32_MAX__
#define INT16_MAX __INT16_MAX__
#define INT8_MAX __INT8_MAX__
#endif

#ifndef INT64_MIN
#define INT64_MIN ((int64_t)~0x7fffffffffffffff)
#define INT32_MIN ((int32_t)~0x7fffffff)
#define INT16_MIN ((int16_t)~0x7fff)
#define INT8_MIN ((int8_t)~0x7f)
#endif

// stdbool
#define BOOL int

// Default Type init
#define DFT(x) ((x){0})

// Array Clear
#define AC(x) memset(x, 0, sizeof(x))
// Array Length
#define AL(x) (sizeof(x) / sizeof(x[0]))
// Array Pair
#define AP(x) x, AL(x)
// Array Modulus
#define AM(arr, id) ((id) % AL(arr))
// Array Slot
#define AS(arr, id) arr[(id) % AL(arr)]
// Array End
#define AE(arr) (arr + sizeof(arr))

#define ABS(x) (x >= 0 ? x : -x)
#define CLAMP(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define OF2(x) (((x - 1) & x) == 0)
#define COMMON_DEBUG 1
#define LOGFMT(...)

// tag data, optionally global
#ifndef DATA
#define DATA
#endif
// group game data for determinism verification
#define GAME DATA __attribute__((section("game")))

// Game build variants
#ifdef RELEASE
#undef RELEASE
enum { RELEASE = 1 };
#else
enum { RELEASE = 0 };
#endif

// Array for reusable type
// index is acquired by _use(); freed by _unuse()
// eid is a stable, generational modulus index into entity array
#define ARR_REUSE(type, max)                                                 \
  _Static_assert(OF2(max), "ARR_USE requires a power of 2");                 \
  GAME int type##D[max];                                                     \
  GAME int type##_usedD;                                                     \
  GAME struct type##S entity_##type##D[max];                                 \
  static struct type##S* type##_use()                                        \
  {                                                                          \
    int it = type##_usedD;                                                   \
    int next = it + 1;                                                       \
    if (next >= max) return &entity_##type##D[0];                            \
    int eid = type##D[it];                                                   \
    if (!eid) {                                                              \
      eid = next;                                                            \
    } else {                                                                 \
      eid += max;                                                            \
    }                                                                        \
    type##D[it] = eid;                                                       \
    type##_usedD = next;                                                     \
    if (COMMON_DEBUG) {                                                      \
      LOGFMT(#type " ALLOC (it %d eid %d) kUsed %d", it, eid, type##_usedD); \
    }                                                                        \
                                                                             \
    struct type##S* ent = &AS(entity_##type##D, eid);                        \
    ent->id = eid;                                                           \
    return ent;                                                              \
  }                                                                          \
  static struct type##S* type##_get(int eid)                                 \
  {                                                                          \
    struct type##S* dflt = entity_##type##D;                                 \
    struct type##S* ent = &AS(entity_##type##D, eid);                        \
    return ent->id == eid ? ent : dflt;                                      \
  }                                                                          \
  static int type##_index(struct type##S* ent)                               \
  {                                                                          \
    return AM(type##D, ent->id);                                             \
  }                                                                          \
  static void type##_unuse(struct type##S* ent)                              \
  {                                                                          \
    int eid = ent->id;                                                       \
    int it = 0;                                                              \
    for (; it < AL(type##D); ++it) {                                         \
      if (type##D[it] == eid) break;                                         \
    }                                                                        \
    if (it < type##_usedD) {                                                 \
      int swap_it = type##_usedD - 1;                                        \
      int swap_eid = type##D[swap_it];                                       \
      type##_usedD -= 1;                                                     \
      if (COMMON_DEBUG) {                                                    \
        LOGFMT(#type                                                         \
               " FREE (it %d swap_it %d) (eid %d vs swap_eid %d) kUsed %d",  \
               it, swap_it, eid, swap_eid, type##_usedD);                    \
      }                                                                      \
      int xchg = eid ^ swap_eid;                                             \
      type##D[swap_it] ^= xchg;                                              \
      type##D[it] ^= xchg;                                                   \
    }                                                                        \
    *ent = DFT(struct type##S);                                              \
  }

#define FOR_EACH(type, body)                                     \
  {                                                              \
    for (int it_index = 0; it_index < AL(type##D); ++it_index) { \
      struct type##S* type = &entity_##type##D[it_index];        \
      if (!type->id) continue;                                   \
      body;                                                      \
    }                                                            \
  }
