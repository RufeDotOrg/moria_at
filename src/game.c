#include "game/common.h"
#include "game/const.h"
#include "game/type.h"
#include "game/var.h"

#include "db/char.c"
#include "db/desc.c"
#include "db/grave.c"
#include "db/mon.c"
#include "db/obj.c"

#include "platform/platform.c"

#ifdef SDL_config_h_
enum { SDL = 1 };
enum { TTY = 0 };
#else
enum { SDL = 0 };
enum { TTY = 1 };
#endif
