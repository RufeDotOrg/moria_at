#include "src/game.c"

int
los()
{
  return 0;
}

#define E(x) \
  case x:    \
    return #x
char*
tval_name(tval)
{
  switch (tval) {
    E(TV_NOTHING);
    E(TV_MISC);
    E(TV_CHEST);
    E(TV_SPIKE);
    E(TV_PROJECTILE);
    E(TV_LIGHT);
    E(TV_LAUNCHER);
    E(TV_HAFTED);
    E(TV_POLEARM);
    E(TV_SWORD);
    E(TV_DIGGING);
    E(TV_BOOTS);
    E(TV_GLOVES);
    E(TV_CLOAK);
    E(TV_HELM);
    E(TV_SHIELD);
    E(TV_HARD_ARMOR);
    E(TV_SOFT_ARMOR);
    E(TV_AMULET);
    E(TV_RING);
    E(TV_STAFF);
    E(TV_WAND);
    E(TV_SCROLL1);
    E(TV_SCROLL2);
    E(TV_POTION1);
    E(TV_POTION2);
    E(TV_FLASK);
    E(TV_FOOD);
    E(TV_MAGIC_BOOK);
    E(TV_PRAYER_BOOK);
    E(TV_MON_PICK_UP);
    E(TV_GOLD);
    E(TV_INVIS_TRAP);
    E(TV_VIS_TRAP);
    E(TV_RUBBLE);
    E(TV_OPEN_DOOR);
    E(TV_CLOSED_DOOR);
    E(TV_UP_STAIR);
    E(TV_DOWN_STAIR);
    E(TV_SECRET_DOOR);
    E(TV_STORE_DOOR);
    E(TV_GLYPH);
    E(TV_PAWN_DOOR);
  }
  return "";
}
char tcharD[4];
char*
tchar_name(tchar)
{
  tcharD[0] = tchar;
  if (tchar == '\\' || tchar == '\'') {
    tcharD[1] = tchar;
    tcharD[2] = 0;
  } else {
    tcharD[1] = 0;
  }
  return tcharD;
}
int
main()
{
  printf("static struct treasureS treasureD[] = {{0},");
  for (int it = 1; it < AL(treasureD); ++it) {
    if (it == MAX_DUNGEON_OBJ) printf("// MAX_DUNGEON_OBJ\n");
    struct treasureS* tr_ptr = &treasureD[it];
    printf(
        "{"
        "\"%s\","
        "0x%08XL,"
        "%s,"
        "'%s', /*  %d*/\n"
        "%d,"
        "%d,"
        "%u,"
        "%d,"
        "%d,"
        "%d,"
        "%d,"
        "%u,"
        "{%u, %u},"
        "%u"
        "},\n",
        tr_ptr->name, tr_ptr->flags, tval_name(tr_ptr->tval),
        tchar_name(tr_ptr->tchar), it, tr_ptr->p1, tr_ptr->cost, tr_ptr->weight,
        tr_ptr->tohit, tr_ptr->todam, tr_ptr->ac, tr_ptr->toac, tr_ptr->subval,
        tr_ptr->damage[0], tr_ptr->damage[1], tr_ptr->level);
  }
  printf("};\n");

  return 0;
}
