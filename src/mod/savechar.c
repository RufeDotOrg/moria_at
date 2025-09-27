// Rufe.org LLC 2022-2025: GPLv3 License

#define BUF(x)    \
  (struct bufS)   \
  {               \
    &x, sizeof(x) \
  }
// Array Buffer
#define ABUF(x)  \
  (struct bufS)  \
  {              \
    x, sizeof(x) \
  }

// load/save
DATA struct bufS save_bufD[] = {
    BUF(countD),        BUF(dun_level),   ABUF(entity_objD), ABUF(invenD),
    ABUF(knownD),       ABUF(maD),        ABUF(objD),        BUF(obj_usedD),
    ABUF(player_hpD),   BUF(rnd_seed),    BUF(town_seed),    BUF(obj_seed),
    BUF(statD),         ABUF(store_objD), BUF(turnD),        BUF(uD),
    ABUF(spell_orderD), ABUF(ui_stateD),
};
DATA int savechar_v000[AL(save_bufD)] = {
    28, 4, 10240, 136, 448, 56, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 100, 0, 0,
};
#define SAVESUM000 16080
DATA int savechar_v001[] = {
    28, 4, 10240, 136, 448, 76, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 116, 32, 0,
};
#define SAVESUM001 16148
DATA int savechar_v002[] = {
    28, 4, 10240, 140, 448, 76, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 128, 32, 0,
};
#define SAVESUM002 16164
DATA int savechar_v003[] = {
    28, 4, 10240, 140, 448,  76, 1024, 4,  160,
    4,  4, 4,     24,  3840, 4,  128,  32, 256,
};
#define SAVESUM003 16420
DATA int savesumD[] = {SAVESUM000, SAVESUM001, SAVESUM002, SAVESUM003};
DATA int* savefieldD[] = {
    savechar_v000,
    savechar_v001,
    savechar_v002,
    savechar_v003,
};
