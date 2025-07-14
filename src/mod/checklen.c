// Rufe.org LLC 2022-2025: GPLv3 License

static void tr_obj_copy(int tidx, struct objS* obj);
void obj_desc(struct objS* obj, int number);
int magic_init();
static int fixed_seed_func(int seed, fn func);

int64_t moria_ocat_num(char* dst, int dstlen, char* objname, int num);
int
obj_checklen()
{
  fixed_seed_func(1337, magic_init);

  int limit = 0;
  struct objS* obj = 0;
  for (int tidx = 1; tidx < AL(treasureD); ++tidx) {
    struct treasureS* tr_ptr = &treasureD[tidx];
    obj = obj_use();
    if (tr_ptr->name) {
      tr_obj_copy(tidx, obj);

      obj_desc(obj, 1);
      limit = MAX(strlen(descD), limit);
      printf("%d) %jd strlen %s\n", tidx, strlen(descD), descD);
      tr_make_known(tr_ptr);

      if (tidx < OBJ_TRAP_BEGIN) {
        obj->idflag = ID_REVEAL;
        obj_desc(obj, 1);
        limit = MAX(strlen(descD), limit);
        printf("%d) %jd strlen %s\n", tidx, strlen(descD), descD);
      }

      if (STACK_ANY & obj->subval) {
        obj_desc(obj, 0);
        limit = MAX(strlen(descD), limit);
        printf("%d) %jd strlen %s\n", tidx, strlen(descD), descD);

        obj_desc(obj, 255);
        limit = MAX(strlen(descD), limit);
        printf("%d) %jd strlen %s\n", tidx, strlen(descD), descD);
      }
    }
    obj_unuse(obj);
  }
  printf("limit %d\n", limit);
  return 0;
}
#define TEST_CHECKLEN 1
