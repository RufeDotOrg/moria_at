#include "game.c"

char msg_cqD[16][80];
char msglen_cqD[16];
uint32_t msg_writeD;

int
main()
{
  char* log;
  uint32_t log_used;

  log = log_used = 0;
  for (int it = 0; it < 32; ++it, ++msg_writeD) {
    AS(msglen_cqD, msg_writeD - 1) = log_used;
    log = AS(msg_cqD, msg_writeD);
    log_used = 0;
    for (int jt = 1; jt < 4; ++jt) {
      log_used += snprintf(log + log_used, 80 - log_used, "%d |", jt * it);
    }
    log[log_used] = 0;
    puts(log);
  }

  for (int it = 0; it < 16; ++it) {
    log = AS(msg_cqD, msg_writeD + it);
    log_used = AS(msglen_cqD, msg_writeD + it);
    printf("[%d] %d) %s\n", msg_writeD + it, log_used, log);
  }
}
