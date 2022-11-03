#include <stdint.h>

#define BOOL int

#define ABS(x) (x >= 0 ? x : -x)
#define AL(x) (sizeof(x) / sizeof(x[0]))
#define AP(x) x, AL(x)
#define AM(arr, id) (id % AL(arr))
#define AS(arr, id) arr[id % AL(arr)]
#define CLAMP(x, min, max) (x < min ? min : x > max ? max : x)
#define DFT(x) ((x){0})
#define OF2(x) (((x - 1) & x) == 0)
#define COMMON_DEBUG 1
#define LOGFMT(...)

// Array for reusable type
// index is acquired by _use(); freed by _unuse()
// eid is a stable, generational modulus index into entity array
#define ARR_REUSE(type, max)                                                 \
  _Static_assert(OF2(max), "ARR_USE requires a power of 2");                 \
  static int type##D[max];                                                   \
  static int type##_usedD;                                                   \
  static struct type##S entity_##type##D[max];                               \
  struct type##S* type##_use()                                               \
  {                                                                          \
    int it = type##_usedD;                                                   \
    if (it >= max) return &entity_##type##D[0];                              \
    int next = it + 1;                                                       \
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
  struct type##S* type##_get(int eid)                                        \
  {                                                                          \
    struct type##S* dflt = entity_##type##D;                                   \
    struct type##S* ent = &AS(entity_##type##D, eid);                        \
    return ent->id == eid ? ent : dflt;                                      \
  }                                                                          \
  int type##_index(struct type##S* ent)                                      \
  {                                                                          \
    return AM(type##D, ent->id);                                             \
  }                                                                          \
  void type##_unuse(struct type##S* ent)                                     \
  {                                                                          \
    int eid = ent->id;                                                       \
    int it = 0;                                                              \
    for (; it < AL(type##D); ++it) {                                         \
      if (type##D[it] == eid) break;                                         \
    }                                                                        \
    if (it < AL(type##D)) {                                                  \
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
