
#include "src/game.c"

int
los()
{
  return 0;
}
char*
attack_string(adesc)
{
  switch (adesc) {
    case 1:
      return ((" hits you."));
    case 2:
      return ((" bites you."));
    case 3:
      return ((" claws you."));
    case 4:
      return ((" stings you."));
    case 5:
      return ((" touches you."));
    case 6:
      return ((" kicks you."));
    case 7:
      return ((" gazes at you."));
    case 8:
      return ((" breathes on you."));
    case 9:
      return ((" spits on you."));
    case 10:
      return ((" makes a horrible wail."));
    case 11:
      return ((" embraces you."));
    case 12:
      return ((" crawls on you."));
    case 13:
      return ((" releases a cloud of spores."));
    case 14:
      return ((" begs you for money."));
    case 15:
      descD[0] = 0;
      return ("You've been slimed!");
    case 16:
      return ((" crushes you."));
    case 17:
      return ((" tramples you."));
    case 18:
      return ((" drools on you."));
    case 19:
      return ((" insults you!"));
    case 99:
      return ((" is repelled."));
  }
  return " hits you.";
}

int
main()
{
  int type[128];
  int type_used = 0;
  uint32_t type_set = 0;

  for (int it = 0; it < AL(creatureD); ++it) {
    struct creatureS* cre = &creatureD[it];

    if ((cre->cdefense & CD_UNDEAD)) {
      printf("%s\n", cre->name);
      for (int jt = 0; jt < AL(cre->attack_list); ++jt) {
        int at = cre->attack_list[jt];

        if (at) {
          int idx = 0;
          while (idx < type_used) {
            if (type[idx] == at) idx = AL(type);
            idx += 1;
          }

          if (idx < AL(type)) {
            type[idx] = at;
            type_used += 1;

            int ato = attackD[at].attack_type - 1;
            if ((type_set & (1 << ato)) == 0) {
              type_set |= (1 << ato);
              printf("adding %d\n", ato + 1);
            }
          }
        }
      }
    }
  }

  for (int it = 0; it < type_used; ++it) {
    printf("%d ", type[it]);
  }
  printf("\n");
  for (int it = 0; it < type_used; ++it) {
    printf("%s ", attack_string(attackD[type[it]].attack_desc));
  }
  printf("\n");
  type_set = 0;
  for (int it = 0; it < type_used; ++it) {
    type_set |= (1 << (attackD[type[it]].attack_type - 1));
  }
  printf("%x\n", type_set);

  printf("not ");
  for (int it = 0; it < 24; ++it) {
    if ((type_set & (1 << (it))) == 0) printf("%d ", it + 1);
  }
  printf("\n");

  if (type_used == AL(type)) printf("query results full\n");
  return 0;
}
