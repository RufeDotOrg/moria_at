
int
obj_checklen()
{
  int limit = 0;
  struct objS* obj = 0;
  for (int it = 1; it < AL(treasureD); ++it) {
    int tidx = it;
    struct treasureS* tr_ptr = &treasureD[tidx];
    obj = obj_use();
    if (tr_ptr->name) {
      tr_make_known(tr_ptr);
      tr_obj_copy(tidx, obj);
      obj->idflag = ID_REVEAL;

      printf("--- treasure %d ---\n", tidx);
      for (int jt = 0; jt < 256; ++jt) {
        obj_desc(obj, jt);
        limit = MAX(strlen(descD), limit);
        printf("%d) strlen %ju %s\n", jt, strlen(descD), descD);
      }
    }
    obj_unuse(obj);
  }
  printf("limit %d\n", limit);
  return 0;
}
#define TEST_CHECKLEN 1
