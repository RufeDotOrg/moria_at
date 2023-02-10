#include <dlfcn.h>

static void* dl_ptr;
void
platform_update()
{
  if (dl_ptr == 0 || dlclose(dl_ptr) == 0) {
    dl_ptr = dlopen("./bin/render", RTLD_NOW);
    // if (!dl_ptr) msg_print(dl_error());
    int (*f_ptr)() = dlsym(dl_ptr, "platform_init");
    f_ptr();
  }
}
