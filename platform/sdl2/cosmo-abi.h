SDL_Thread *abi_SDL_CreateThread(SDL_ThreadFunction a, const char *b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateThread");
    if (!IsWindows()) {
        SDL_Thread *(*local)(SDL_ThreadFunction a, const char *b, void *c) = addr;
        return local(a, b, c);
    } else {
        SDL_Thread *__attribute__((__ms_abi__)) (*local)(SDL_ThreadFunction a, const char *b, void *c) = addr;
        return local(a, b, c);
    }
}
SDL_RWops *abi_SDL_RWFromFP(void *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWFromFP");
    if (!IsWindows()) {
        SDL_RWops *(*local)(void *a, SDL_bool b) = addr;
        return local(a, b);
    } else {
        SDL_RWops *__attribute__((__ms_abi__)) (*local)(void *a, SDL_bool b) = addr;
        return local(a, b);
    }
}
int abi_SDL_Init(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Init");
    if (!IsWindows()) {
        int (*local)(Uint32 a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
int abi_SDL_InitSubSystem(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_InitSubSystem");
    if (!IsWindows()) {
        int (*local)(Uint32 a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
void abi_SDL_QuitSubSystem(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_QuitSubSystem");
    if (!IsWindows()) {
        void (*local)(Uint32 a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        local(a);
    }
}
Uint32 abi_SDL_WasInit(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_WasInit");
    if (!IsWindows()) {
        Uint32 (*local)(Uint32 a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
void abi_SDL_Quit(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_Quit");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_AssertState abi_SDL_ReportAssertion(SDL_AssertData *a, const char *b, const char *c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReportAssertion");
    if (!IsWindows()) {
        SDL_AssertState (*local)(SDL_AssertData * a, const char *b, const char *c, int d) = addr;
        return local(a, b, c, d);
    } else {
        SDL_AssertState __attribute__((__ms_abi__)) (*local)(SDL_AssertData * a, const char *b, const char *c, int d) = addr;
        return local(a, b, c, d);
    }
}
void abi_SDL_SetAssertionHandler(SDL_AssertionHandler a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetAssertionHandler");
    if (!IsWindows()) {
        void (*local)(SDL_AssertionHandler a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AssertionHandler a, void *b) = addr;
        local(a, b);
    }
}
const SDL_AssertData *abi_SDL_GetAssertionReport(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAssertionReport");
    if (!IsWindows()) {
        const SDL_AssertData *(*local)(void) = addr;
        return local();
    } else {
        const SDL_AssertData *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_ResetAssertionReport(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ResetAssertionReport");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_AtomicTryLock(SDL_SpinLock *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicTryLock");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_SpinLock * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_SpinLock * a) = addr;
        return local(a);
    }
}
void abi_SDL_AtomicLock(SDL_SpinLock *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicLock");
    if (!IsWindows()) {
        void (*local)(SDL_SpinLock * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_SpinLock * a) = addr;
        local(a);
    }
}
void abi_SDL_AtomicUnlock(SDL_SpinLock *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicUnlock");
    if (!IsWindows()) {
        void (*local)(SDL_SpinLock * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_SpinLock * a) = addr;
        local(a);
    }
}
SDL_bool abi_SDL_AtomicCAS(SDL_atomic_t *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicCAS");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_atomic_t * a, int b, int c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_atomic_t * a, int b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_AtomicSet(SDL_atomic_t *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicSet");
    if (!IsWindows()) {
        int (*local)(SDL_atomic_t * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_atomic_t * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_AtomicGet(SDL_atomic_t *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicGet");
    if (!IsWindows()) {
        int (*local)(SDL_atomic_t * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_atomic_t * a) = addr;
        return local(a);
    }
}
int abi_SDL_AtomicAdd(SDL_atomic_t *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicAdd");
    if (!IsWindows()) {
        int (*local)(SDL_atomic_t * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_atomic_t * a, int b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_AtomicCASPtr(void **a, void *b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicCASPtr");
    if (!IsWindows()) {
        SDL_bool (*local)(void **a, void *b, void *c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void **a, void *b, void *c) = addr;
        return local(a, b, c);
    }
}
void *abi_SDL_AtomicSetPtr(void **a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicSetPtr");
    if (!IsWindows()) {
        void *(*local)(void **a, void *b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(void **a, void *b) = addr;
        return local(a, b);
    }
}
void *abi_SDL_AtomicGetPtr(void **a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AtomicGetPtr");
    if (!IsWindows()) {
        void *(*local)(void **a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(void **a) = addr;
        return local(a);
    }
}
int abi_SDL_GetNumAudioDrivers(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumAudioDrivers");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_GetAudioDriver(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAudioDriver");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_AudioInit(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioInit");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void abi_SDL_AudioQuit(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioQuit");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
const char *abi_SDL_GetCurrentAudioDriver(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCurrentAudioDriver");
    if (!IsWindows()) {
        const char *(*local)(void) = addr;
        return local();
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_OpenAudio(SDL_AudioSpec *a, SDL_AudioSpec *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_OpenAudio");
    if (!IsWindows()) {
        int (*local)(SDL_AudioSpec * a, SDL_AudioSpec * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioSpec * a, SDL_AudioSpec * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetNumAudioDevices(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumAudioDevices");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GetAudioDeviceName(int a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAudioDeviceName");
    if (!IsWindows()) {
        const char *(*local)(int a, int b) = addr;
        return local(a, b);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a, int b) = addr;
        return local(a, b);
    }
}
SDL_AudioDeviceID abi_SDL_OpenAudioDevice(const char *a, int b, const SDL_AudioSpec *c, SDL_AudioSpec *d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_OpenAudioDevice");
    if (!IsWindows()) {
        SDL_AudioDeviceID (*local)(const char *a, int b, const SDL_AudioSpec *c, SDL_AudioSpec *d, int e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_AudioDeviceID __attribute__((__ms_abi__)) (*local)(const char *a, int b, const SDL_AudioSpec *c, SDL_AudioSpec *d, int e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_AudioStatus abi_SDL_GetAudioStatus(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAudioStatus");
    if (!IsWindows()) {
        SDL_AudioStatus (*local)(void) = addr;
        return local();
    } else {
        SDL_AudioStatus __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_AudioStatus abi_SDL_GetAudioDeviceStatus(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAudioDeviceStatus");
    if (!IsWindows()) {
        SDL_AudioStatus (*local)(SDL_AudioDeviceID a) = addr;
        return local(a);
    } else {
        SDL_AudioStatus __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        return local(a);
    }
}
void abi_SDL_PauseAudio(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_PauseAudio");
    if (!IsWindows()) {
        void (*local)(int a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(int a) = addr;
        local(a);
    }
}
void abi_SDL_PauseAudioDevice(SDL_AudioDeviceID a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_PauseAudioDevice");
    if (!IsWindows()) {
        void (*local)(SDL_AudioDeviceID a, int b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a, int b) = addr;
        local(a, b);
    }
}
SDL_AudioSpec *abi_SDL_LoadWAV_RW(SDL_RWops *a, int b, SDL_AudioSpec *c, Uint8 **d, Uint32 *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadWAV_RW");
    if (!IsWindows()) {
        SDL_AudioSpec *(*local)(SDL_RWops * a, int b, SDL_AudioSpec *c, Uint8 **d, Uint32 *e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_AudioSpec *__attribute__((__ms_abi__)) (*local)(SDL_RWops * a, int b, SDL_AudioSpec *c, Uint8 **d, Uint32 *e) = addr;
        return local(a, b, c, d, e);
    }
}
void abi_SDL_FreeWAV(Uint8 *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeWAV");
    if (!IsWindows()) {
        void (*local)(Uint8 * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint8 * a) = addr;
        local(a);
    }
}
int abi_SDL_BuildAudioCVT(SDL_AudioCVT *a, SDL_AudioFormat b, Uint8 c, int d, SDL_AudioFormat e, Uint8 f, int g)
{
    void *addr = cosmo_dlsym(libD, "SDL_BuildAudioCVT");
    if (!IsWindows()) {
        int (*local)(SDL_AudioCVT * a, SDL_AudioFormat b, Uint8 c, int d, SDL_AudioFormat e, Uint8 f, int g) = addr;
        return local(a, b, c, d, e, f, g);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioCVT * a, SDL_AudioFormat b, Uint8 c, int d, SDL_AudioFormat e, Uint8 f, int g) = addr;
        return local(a, b, c, d, e, f, g);
    }
}
int abi_SDL_ConvertAudio(SDL_AudioCVT *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ConvertAudio");
    if (!IsWindows()) {
        int (*local)(SDL_AudioCVT * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioCVT * a) = addr;
        return local(a);
    }
}
void abi_SDL_MixAudio(Uint8 *a, const Uint8 *b, Uint32 c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_MixAudio");
    if (!IsWindows()) {
        void (*local)(Uint8 * a, const Uint8 *b, Uint32 c, int d) = addr;
        local(a, b, c, d);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint8 * a, const Uint8 *b, Uint32 c, int d) = addr;
        local(a, b, c, d);
    }
}
void abi_SDL_MixAudioFormat(Uint8 *a, const Uint8 *b, SDL_AudioFormat c, Uint32 d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_MixAudioFormat");
    if (!IsWindows()) {
        void (*local)(Uint8 * a, const Uint8 *b, SDL_AudioFormat c, Uint32 d, int e) = addr;
        local(a, b, c, d, e);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint8 * a, const Uint8 *b, SDL_AudioFormat c, Uint32 d, int e) = addr;
        local(a, b, c, d, e);
    }
}
void abi_SDL_LockAudio(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockAudio");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_LockAudioDevice(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockAudioDevice");
    if (!IsWindows()) {
        void (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    }
}
void abi_SDL_UnlockAudio(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockAudio");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_UnlockAudioDevice(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockAudioDevice");
    if (!IsWindows()) {
        void (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    }
}
void abi_SDL_CloseAudio(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_CloseAudio");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_CloseAudioDevice(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CloseAudioDevice");
    if (!IsWindows()) {
        void (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    }
}
int abi_SDL_SetClipboardText(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetClipboardText");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_GetClipboardText(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetClipboardText");
    if (!IsWindows()) {
        char *(*local)(void) = addr;
        return local();
    } else {
        char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasClipboardText(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasClipboardText");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetCPUCount(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCPUCount");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetCPUCacheLineSize(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCPUCacheLineSize");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasRDTSC(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasRDTSC");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasAltiVec(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasAltiVec");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasMMX(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasMMX");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_Has3DNow(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_Has3DNow");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasSSE(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSSE");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasSSE2(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSSE2");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasSSE3(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSSE3");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasSSE41(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSSE41");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasSSE42(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSSE42");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetSystemRAM(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetSystemRAM");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_GetError(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetError");
    if (!IsWindows()) {
        const char *(*local)(void) = addr;
        return local();
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_ClearError(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ClearError");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_Error(SDL_errorcode a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Error");
    if (!IsWindows()) {
        int (*local)(SDL_errorcode a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_errorcode a) = addr;
        return local(a);
    }
}
void abi_SDL_PumpEvents(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_PumpEvents");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_PeepEvents(SDL_Event *a, int b, SDL_eventaction c, Uint32 d, Uint32 e)
{
    void *addr = cosmo_dlsym(libD, "SDL_PeepEvents");
    if (!IsWindows()) {
        int (*local)(SDL_Event * a, int b, SDL_eventaction c, Uint32 d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Event * a, int b, SDL_eventaction c, Uint32 d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_bool abi_SDL_HasEvent(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasEvent");
    if (!IsWindows()) {
        SDL_bool (*local)(Uint32 a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasEvents(Uint32 a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasEvents");
    if (!IsWindows()) {
        SDL_bool (*local)(Uint32 a, Uint32 b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(Uint32 a, Uint32 b) = addr;
        return local(a, b);
    }
}
void abi_SDL_FlushEvent(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FlushEvent");
    if (!IsWindows()) {
        void (*local)(Uint32 a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        local(a);
    }
}
void abi_SDL_FlushEvents(Uint32 a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_FlushEvents");
    if (!IsWindows()) {
        void (*local)(Uint32 a, Uint32 b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a, Uint32 b) = addr;
        local(a, b);
    }
}
int abi_SDL_PollEvent(SDL_Event *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_PollEvent");
    if (!IsWindows()) {
        int (*local)(SDL_Event * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Event * a) = addr;
        return local(a);
    }
}
int abi_SDL_WaitEvent(SDL_Event *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_WaitEvent");
    if (!IsWindows()) {
        int (*local)(SDL_Event * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Event * a) = addr;
        return local(a);
    }
}
int abi_SDL_WaitEventTimeout(SDL_Event *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WaitEventTimeout");
    if (!IsWindows()) {
        int (*local)(SDL_Event * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Event * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_PushEvent(SDL_Event *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_PushEvent");
    if (!IsWindows()) {
        int (*local)(SDL_Event * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Event * a) = addr;
        return local(a);
    }
}
void abi_SDL_SetEventFilter(SDL_EventFilter a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetEventFilter");
    if (!IsWindows()) {
        void (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    }
}
SDL_bool abi_SDL_GetEventFilter(SDL_EventFilter *a, void **b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetEventFilter");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_EventFilter * a, void **b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_EventFilter * a, void **b) = addr;
        return local(a, b);
    }
}
void abi_SDL_AddEventWatch(SDL_EventFilter a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_AddEventWatch");
    if (!IsWindows()) {
        void (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    }
}
void abi_SDL_DelEventWatch(SDL_EventFilter a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_DelEventWatch");
    if (!IsWindows()) {
        void (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    }
}
void abi_SDL_FilterEvents(SDL_EventFilter a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_FilterEvents");
    if (!IsWindows()) {
        void (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_EventFilter a, void *b) = addr;
        local(a, b);
    }
}
Uint8 abi_SDL_EventState(Uint32 a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_EventState");
    if (!IsWindows()) {
        Uint8 (*local)(Uint32 a, int b) = addr;
        return local(a, b);
    } else {
        Uint8 __attribute__((__ms_abi__)) (*local)(Uint32 a, int b) = addr;
        return local(a, b);
    }
}
Uint32 abi_SDL_RegisterEvents(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RegisterEvents");
    if (!IsWindows()) {
        Uint32 (*local)(int a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
char *abi_SDL_GetBasePath(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetBasePath");
    if (!IsWindows()) {
        char *(*local)(void) = addr;
        return local();
    } else {
        char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
char *abi_SDL_GetPrefPath(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPrefPath");
    if (!IsWindows()) {
        char *(*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerAddMapping(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerAddMapping");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_GameControllerMappingForGUID(SDL_JoystickGUID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerMappingForGUID");
    if (!IsWindows()) {
        char *(*local)(SDL_JoystickGUID a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(SDL_JoystickGUID a) = addr;
        return local(a);
    }
}
char *abi_SDL_GameControllerMapping(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerMapping");
    if (!IsWindows()) {
        char *(*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_IsGameController(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsGameController");
    if (!IsWindows()) {
        SDL_bool (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GameControllerNameForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerNameForIndex");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_GameController *abi_SDL_GameControllerOpen(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerOpen");
    if (!IsWindows()) {
        SDL_GameController *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_GameController *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GameControllerName(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerName");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GameControllerGetAttached(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetAttached");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_Joystick *abi_SDL_GameControllerGetJoystick(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetJoystick");
    if (!IsWindows()) {
        SDL_Joystick *(*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_Joystick *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
int abi_SDL_GameControllerEventState(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerEventState");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void abi_SDL_GameControllerUpdate(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerUpdate");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_GameControllerAxis abi_SDL_GameControllerGetAxisFromString(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetAxisFromString");
    if (!IsWindows()) {
        SDL_GameControllerAxis (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_GameControllerAxis __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetStringForAxis");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameControllerAxis a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameControllerAxis a) = addr;
        return local(a);
    }
}
SDL_GameControllerButtonBind abi_SDL_GameControllerGetBindForAxis(SDL_GameController *a, SDL_GameControllerAxis b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetBindForAxis");
    if (!IsWindows()) {
        SDL_GameControllerButtonBind (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    } else {
        SDL_GameControllerButtonBind __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    }
}
Sint16 abi_SDL_GameControllerGetAxis(SDL_GameController *a, SDL_GameControllerAxis b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetAxis");
    if (!IsWindows()) {
        Sint16 (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    } else {
        Sint16 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    }
}
SDL_GameControllerButton abi_SDL_GameControllerGetButtonFromString(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetButtonFromString");
    if (!IsWindows()) {
        SDL_GameControllerButton (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_GameControllerButton __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GameControllerGetStringForButton(SDL_GameControllerButton a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetStringForButton");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameControllerButton a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameControllerButton a) = addr;
        return local(a);
    }
}
SDL_GameControllerButtonBind abi_SDL_GameControllerGetBindForButton(SDL_GameController *a, SDL_GameControllerButton b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetBindForButton");
    if (!IsWindows()) {
        SDL_GameControllerButtonBind (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    } else {
        SDL_GameControllerButtonBind __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    }
}
Uint8 abi_SDL_GameControllerGetButton(SDL_GameController *a, SDL_GameControllerButton b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetButton");
    if (!IsWindows()) {
        Uint8 (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    } else {
        Uint8 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    }
}
void abi_SDL_GameControllerClose(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerClose");
    if (!IsWindows()) {
        void (*local)(SDL_GameController * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        local(a);
    }
}
int abi_SDL_RecordGesture(SDL_TouchID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RecordGesture");
    if (!IsWindows()) {
        int (*local)(SDL_TouchID a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_TouchID a) = addr;
        return local(a);
    }
}
int abi_SDL_SaveAllDollarTemplates(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SaveAllDollarTemplates");
    if (!IsWindows()) {
        int (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
int abi_SDL_SaveDollarTemplate(SDL_GestureID a, SDL_RWops *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SaveDollarTemplate");
    if (!IsWindows()) {
        int (*local)(SDL_GestureID a, SDL_RWops * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GestureID a, SDL_RWops * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_LoadDollarTemplates(SDL_TouchID a, SDL_RWops *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadDollarTemplates");
    if (!IsWindows()) {
        int (*local)(SDL_TouchID a, SDL_RWops * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_TouchID a, SDL_RWops * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_NumHaptics(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_NumHaptics");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_HapticName(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticName");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_Haptic *abi_SDL_HapticOpen(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticOpen");
    if (!IsWindows()) {
        SDL_Haptic *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_Haptic *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticOpened(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticOpened");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticIndex(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticIndex");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_MouseIsHaptic(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_MouseIsHaptic");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_Haptic *abi_SDL_HapticOpenFromMouse(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticOpenFromMouse");
    if (!IsWindows()) {
        SDL_Haptic *(*local)(void) = addr;
        return local();
    } else {
        SDL_Haptic *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_JoystickIsHaptic(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickIsHaptic");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_Haptic *abi_SDL_HapticOpenFromJoystick(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticOpenFromJoystick");
    if (!IsWindows()) {
        SDL_Haptic *(*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_Haptic *__attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
void abi_SDL_HapticClose(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticClose");
    if (!IsWindows()) {
        void (*local)(SDL_Haptic * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        local(a);
    }
}
int abi_SDL_HapticNumEffects(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticNumEffects");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticNumEffectsPlaying(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticNumEffectsPlaying");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
unsigned int abi_SDL_HapticQuery(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticQuery");
    if (!IsWindows()) {
        unsigned int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        unsigned int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticNumAxes(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticNumAxes");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticEffectSupported(SDL_Haptic *a, SDL_HapticEffect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticEffectSupported");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, SDL_HapticEffect * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, SDL_HapticEffect * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_HapticNewEffect(SDL_Haptic *a, SDL_HapticEffect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticNewEffect");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, SDL_HapticEffect * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, SDL_HapticEffect * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_HapticUpdateEffect(SDL_Haptic *a, int b, SDL_HapticEffect *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticUpdateEffect");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b, SDL_HapticEffect *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b, SDL_HapticEffect *c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_HapticRunEffect(SDL_Haptic *a, int b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticRunEffect");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_HapticStopEffect(SDL_Haptic *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticStopEffect");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    }
}
void abi_SDL_HapticDestroyEffect(SDL_Haptic *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticDestroyEffect");
    if (!IsWindows()) {
        void (*local)(SDL_Haptic * a, int b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b) = addr;
        local(a, b);
    }
}
int abi_SDL_HapticGetEffectStatus(SDL_Haptic *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticGetEffectStatus");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_HapticSetGain(SDL_Haptic *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticSetGain");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_HapticSetAutocenter(SDL_Haptic *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticSetAutocenter");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_HapticPause(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticPause");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticUnpause(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticUnpause");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticStopAll(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticStopAll");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticRumbleSupported(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticRumbleSupported");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticRumbleInit(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticRumbleInit");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
int abi_SDL_HapticRumblePlay(SDL_Haptic *a, float b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticRumblePlay");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a, float b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a, float b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_HapticRumbleStop(SDL_Haptic *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HapticRumbleStop");
    if (!IsWindows()) {
        int (*local)(SDL_Haptic * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Haptic * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_SetHintWithPriority(const char *a, const char *b, SDL_HintPriority c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetHintWithPriority");
    if (!IsWindows()) {
        SDL_bool (*local)(const char *a, const char *b, SDL_HintPriority c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const char *a, const char *b, SDL_HintPriority c) = addr;
        return local(a, b, c);
    }
}
SDL_bool abi_SDL_SetHint(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetHint");
    if (!IsWindows()) {
        SDL_bool (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
const char *abi_SDL_GetHint(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetHint");
    if (!IsWindows()) {
        const char *(*local)(const char *a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void abi_SDL_AddHintCallback(const char *a, SDL_HintCallback b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AddHintCallback");
    if (!IsWindows()) {
        void (*local)(const char *a, SDL_HintCallback b, void *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(const char *a, SDL_HintCallback b, void *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_DelHintCallback(const char *a, SDL_HintCallback b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_DelHintCallback");
    if (!IsWindows()) {
        void (*local)(const char *a, SDL_HintCallback b, void *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(const char *a, SDL_HintCallback b, void *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_ClearHints(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ClearHints");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_NumJoysticks(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_NumJoysticks");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_JoystickNameForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickNameForIndex");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_Joystick *abi_SDL_JoystickOpen(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickOpen");
    if (!IsWindows()) {
        SDL_Joystick *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_Joystick *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_JoystickName(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickName");
    if (!IsWindows()) {
        const char *(*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_JoystickGUID abi_SDL_JoystickGetDeviceGUID(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceGUID");
    if (!IsWindows()) {
        SDL_JoystickGUID (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_JoystickGUID __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_JoystickGUID abi_SDL_JoystickGetGUID(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetGUID");
    if (!IsWindows()) {
        SDL_JoystickGUID (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_JoystickGUID __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
void abi_SDL_JoystickGetGUIDString(SDL_JoystickGUID a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetGUIDString");
    if (!IsWindows()) {
        void (*local)(SDL_JoystickGUID a, char *b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_JoystickGUID a, char *b, int c) = addr;
        local(a, b, c);
    }
}
SDL_JoystickGUID abi_SDL_JoystickGetGUIDFromString(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetGUIDFromString");
    if (!IsWindows()) {
        SDL_JoystickGUID (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_JoystickGUID __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_JoystickGetAttached(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetAttached");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_JoystickID abi_SDL_JoystickInstanceID(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickInstanceID");
    if (!IsWindows()) {
        SDL_JoystickID (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_JoystickID __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickNumAxes(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickNumAxes");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickNumBalls(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickNumBalls");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickNumHats(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickNumHats");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickNumButtons(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickNumButtons");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
void abi_SDL_JoystickUpdate(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickUpdate");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_JoystickEventState(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickEventState");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
Sint16 abi_SDL_JoystickGetAxis(SDL_Joystick *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetAxis");
    if (!IsWindows()) {
        Sint16 (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    } else {
        Sint16 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    }
}
Uint8 abi_SDL_JoystickGetHat(SDL_Joystick *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetHat");
    if (!IsWindows()) {
        Uint8 (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    } else {
        Uint8 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_JoystickGetBall(SDL_Joystick *a, int b, int *c, int *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetBall");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, int b, int *c, int *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b, int *c, int *d) = addr;
        return local(a, b, c, d);
    }
}
Uint8 abi_SDL_JoystickGetButton(SDL_Joystick *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetButton");
    if (!IsWindows()) {
        Uint8 (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    } else {
        Uint8 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b) = addr;
        return local(a, b);
    }
}
void abi_SDL_JoystickClose(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickClose");
    if (!IsWindows()) {
        void (*local)(SDL_Joystick * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        local(a);
    }
}
SDL_Window *abi_SDL_GetKeyboardFocus(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetKeyboardFocus");
    if (!IsWindows()) {
        SDL_Window *(*local)(void) = addr;
        return local();
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const Uint8 *abi_SDL_GetKeyboardState(int *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetKeyboardState");
    if (!IsWindows()) {
        const Uint8 *(*local)(int *a) = addr;
        return local(a);
    } else {
        const Uint8 *__attribute__((__ms_abi__)) (*local)(int *a) = addr;
        return local(a);
    }
}
SDL_Keymod abi_SDL_GetModState(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetModState");
    if (!IsWindows()) {
        SDL_Keymod (*local)(void) = addr;
        return local();
    } else {
        SDL_Keymod __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_SetModState(SDL_Keymod a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetModState");
    if (!IsWindows()) {
        void (*local)(SDL_Keymod a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Keymod a) = addr;
        local(a);
    }
}
SDL_Keycode abi_SDL_GetKeyFromScancode(SDL_Scancode a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetKeyFromScancode");
    if (!IsWindows()) {
        SDL_Keycode (*local)(SDL_Scancode a) = addr;
        return local(a);
    } else {
        SDL_Keycode __attribute__((__ms_abi__)) (*local)(SDL_Scancode a) = addr;
        return local(a);
    }
}
SDL_Scancode abi_SDL_GetScancodeFromKey(SDL_Keycode a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetScancodeFromKey");
    if (!IsWindows()) {
        SDL_Scancode (*local)(SDL_Keycode a) = addr;
        return local(a);
    } else {
        SDL_Scancode __attribute__((__ms_abi__)) (*local)(SDL_Keycode a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GetScancodeName(SDL_Scancode a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetScancodeName");
    if (!IsWindows()) {
        const char *(*local)(SDL_Scancode a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Scancode a) = addr;
        return local(a);
    }
}
SDL_Scancode abi_SDL_GetScancodeFromName(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetScancodeFromName");
    if (!IsWindows()) {
        SDL_Scancode (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_Scancode __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GetKeyName(SDL_Keycode a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetKeyName");
    if (!IsWindows()) {
        const char *(*local)(SDL_Keycode a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Keycode a) = addr;
        return local(a);
    }
}
SDL_Keycode abi_SDL_GetKeyFromName(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetKeyFromName");
    if (!IsWindows()) {
        SDL_Keycode (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_Keycode __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void abi_SDL_StartTextInput(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_StartTextInput");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_IsTextInputActive(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsTextInputActive");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_StopTextInput(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_StopTextInput");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_SetTextInputRect(const SDL_Rect *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextInputRect");
    if (!IsWindows()) {
        void (*local)(const SDL_Rect *a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a) = addr;
        local(a);
    }
}
SDL_bool abi_SDL_HasScreenKeyboardSupport(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasScreenKeyboardSupport");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_IsScreenKeyboardShown(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsScreenKeyboardShown");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
void *abi_SDL_LoadObject(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadObject");
    if (!IsWindows()) {
        void *(*local)(const char *a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void *abi_SDL_LoadFunction(void *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadFunction");
    if (!IsWindows()) {
        void *(*local)(void *a, const char *b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(void *a, const char *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_UnloadObject(void *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnloadObject");
    if (!IsWindows()) {
        void (*local)(void *a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(void *a) = addr;
        local(a);
    }
}
void abi_SDL_LogSetAllPriority(SDL_LogPriority a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogSetAllPriority");
    if (!IsWindows()) {
        void (*local)(SDL_LogPriority a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_LogPriority a) = addr;
        local(a);
    }
}
void abi_SDL_LogSetPriority(int a, SDL_LogPriority b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogSetPriority");
    if (!IsWindows()) {
        void (*local)(int a, SDL_LogPriority b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(int a, SDL_LogPriority b) = addr;
        local(a, b);
    }
}
SDL_LogPriority abi_SDL_LogGetPriority(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogGetPriority");
    if (!IsWindows()) {
        SDL_LogPriority (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_LogPriority __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void abi_SDL_LogResetPriorities(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogResetPriorities");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_LogMessageV(int a, SDL_LogPriority b, const char *c, va_list d)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogMessageV");
    if (!IsWindows()) {
        void (*local)(int a, SDL_LogPriority b, const char *c, va_list d) = addr;
        local(a, b, c, d);
    } else {
        void __attribute__((__ms_abi__)) (*local)(int a, SDL_LogPriority b, const char *c, va_list d) = addr;
        local(a, b, c, d);
    }
}
void abi_SDL_LogGetOutputFunction(SDL_LogOutputFunction *a, void **b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogGetOutputFunction");
    if (!IsWindows()) {
        void (*local)(SDL_LogOutputFunction * a, void **b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_LogOutputFunction * a, void **b) = addr;
        local(a, b);
    }
}
void abi_SDL_LogSetOutputFunction(SDL_LogOutputFunction a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LogSetOutputFunction");
    if (!IsWindows()) {
        void (*local)(SDL_LogOutputFunction a, void *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_LogOutputFunction a, void *b) = addr;
        local(a, b);
    }
}
void abi_SDL_SetMainReady(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetMainReady");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_ShowMessageBox(const SDL_MessageBoxData *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_ShowMessageBox");
    if (!IsWindows()) {
        int (*local)(const SDL_MessageBoxData *a, int *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const SDL_MessageBoxData *a, int *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_ShowSimpleMessageBox(Uint32 a, const char *b, const char *c, SDL_Window *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_ShowSimpleMessageBox");
    if (!IsWindows()) {
        int (*local)(Uint32 a, const char *b, const char *c, SDL_Window *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(Uint32 a, const char *b, const char *c, SDL_Window *d) = addr;
        return local(a, b, c, d);
    }
}
SDL_Window *abi_SDL_GetMouseFocus(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetMouseFocus");
    if (!IsWindows()) {
        SDL_Window *(*local)(void) = addr;
        return local();
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
Uint32 abi_SDL_GetMouseState(int *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetMouseState");
    if (!IsWindows()) {
        Uint32 (*local)(int *a, int *b) = addr;
        return local(a, b);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(int *a, int *b) = addr;
        return local(a, b);
    }
}
Uint32 abi_SDL_GetRelativeMouseState(int *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRelativeMouseState");
    if (!IsWindows()) {
        Uint32 (*local)(int *a, int *b) = addr;
        return local(a, b);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(int *a, int *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_WarpMouseInWindow(SDL_Window *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_WarpMouseInWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    }
}
int abi_SDL_SetRelativeMouseMode(SDL_bool a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetRelativeMouseMode");
    if (!IsWindows()) {
        int (*local)(SDL_bool a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_bool a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GetRelativeMouseMode(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRelativeMouseMode");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_Cursor *abi_SDL_CreateCursor(const Uint8 *a, const Uint8 *b, int c, int d, int e, int f)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateCursor");
    if (!IsWindows()) {
        SDL_Cursor *(*local)(const Uint8 *a, const Uint8 *b, int c, int d, int e, int f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_Cursor *__attribute__((__ms_abi__)) (*local)(const Uint8 *a, const Uint8 *b, int c, int d, int e, int f) = addr;
        return local(a, b, c, d, e, f);
    }
}
SDL_Cursor *abi_SDL_CreateColorCursor(SDL_Surface *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateColorCursor");
    if (!IsWindows()) {
        SDL_Cursor *(*local)(SDL_Surface * a, int b, int c) = addr;
        return local(a, b, c);
    } else {
        SDL_Cursor *__attribute__((__ms_abi__)) (*local)(SDL_Surface * a, int b, int c) = addr;
        return local(a, b, c);
    }
}
SDL_Cursor *abi_SDL_CreateSystemCursor(SDL_SystemCursor a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateSystemCursor");
    if (!IsWindows()) {
        SDL_Cursor *(*local)(SDL_SystemCursor a) = addr;
        return local(a);
    } else {
        SDL_Cursor *__attribute__((__ms_abi__)) (*local)(SDL_SystemCursor a) = addr;
        return local(a);
    }
}
void abi_SDL_SetCursor(SDL_Cursor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetCursor");
    if (!IsWindows()) {
        void (*local)(SDL_Cursor * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Cursor * a) = addr;
        local(a);
    }
}
SDL_Cursor *abi_SDL_GetCursor(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCursor");
    if (!IsWindows()) {
        SDL_Cursor *(*local)(void) = addr;
        return local();
    } else {
        SDL_Cursor *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_Cursor *abi_SDL_GetDefaultCursor(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDefaultCursor");
    if (!IsWindows()) {
        SDL_Cursor *(*local)(void) = addr;
        return local();
    } else {
        SDL_Cursor *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_FreeCursor(SDL_Cursor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeCursor");
    if (!IsWindows()) {
        void (*local)(SDL_Cursor * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Cursor * a) = addr;
        local(a);
    }
}
int abi_SDL_ShowCursor(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ShowCursor");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_mutex *abi_SDL_CreateMutex(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateMutex");
    if (!IsWindows()) {
        SDL_mutex *(*local)(void) = addr;
        return local();
    } else {
        SDL_mutex *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_LockMutex(SDL_mutex *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockMutex");
    if (!IsWindows()) {
        int (*local)(SDL_mutex * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_mutex * a) = addr;
        return local(a);
    }
}
int abi_SDL_TryLockMutex(SDL_mutex *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_TryLockMutex");
    if (!IsWindows()) {
        int (*local)(SDL_mutex * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_mutex * a) = addr;
        return local(a);
    }
}
int abi_SDL_UnlockMutex(SDL_mutex *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockMutex");
    if (!IsWindows()) {
        int (*local)(SDL_mutex * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_mutex * a) = addr;
        return local(a);
    }
}
void abi_SDL_DestroyMutex(SDL_mutex *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyMutex");
    if (!IsWindows()) {
        void (*local)(SDL_mutex * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_mutex * a) = addr;
        local(a);
    }
}
SDL_sem *abi_SDL_CreateSemaphore(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateSemaphore");
    if (!IsWindows()) {
        SDL_sem *(*local)(Uint32 a) = addr;
        return local(a);
    } else {
        SDL_sem *__attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
void abi_SDL_DestroySemaphore(SDL_sem *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroySemaphore");
    if (!IsWindows()) {
        void (*local)(SDL_sem * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_sem * a) = addr;
        local(a);
    }
}
int abi_SDL_SemWait(SDL_sem *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SemWait");
    if (!IsWindows()) {
        int (*local)(SDL_sem * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_sem * a) = addr;
        return local(a);
    }
}
int abi_SDL_SemTryWait(SDL_sem *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SemTryWait");
    if (!IsWindows()) {
        int (*local)(SDL_sem * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_sem * a) = addr;
        return local(a);
    }
}
int abi_SDL_SemWaitTimeout(SDL_sem *a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SemWaitTimeout");
    if (!IsWindows()) {
        int (*local)(SDL_sem * a, Uint32 b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_sem * a, Uint32 b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SemPost(SDL_sem *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SemPost");
    if (!IsWindows()) {
        int (*local)(SDL_sem * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_sem * a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_SemValue(SDL_sem *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SemValue");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_sem * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_sem * a) = addr;
        return local(a);
    }
}
SDL_cond *abi_SDL_CreateCond(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateCond");
    if (!IsWindows()) {
        SDL_cond *(*local)(void) = addr;
        return local();
    } else {
        SDL_cond *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_DestroyCond(SDL_cond *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyCond");
    if (!IsWindows()) {
        void (*local)(SDL_cond * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_cond * a) = addr;
        local(a);
    }
}
int abi_SDL_CondSignal(SDL_cond *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CondSignal");
    if (!IsWindows()) {
        int (*local)(SDL_cond * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_cond * a) = addr;
        return local(a);
    }
}
int abi_SDL_CondBroadcast(SDL_cond *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CondBroadcast");
    if (!IsWindows()) {
        int (*local)(SDL_cond * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_cond * a) = addr;
        return local(a);
    }
}
int abi_SDL_CondWait(SDL_cond *a, SDL_mutex *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_CondWait");
    if (!IsWindows()) {
        int (*local)(SDL_cond * a, SDL_mutex * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_cond * a, SDL_mutex * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_CondWaitTimeout(SDL_cond *a, SDL_mutex *b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_CondWaitTimeout");
    if (!IsWindows()) {
        int (*local)(SDL_cond * a, SDL_mutex * b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_cond * a, SDL_mutex * b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
const char *abi_SDL_GetPixelFormatName(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPixelFormatName");
    if (!IsWindows()) {
        const char *(*local)(Uint32 a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_PixelFormatEnumToMasks(Uint32 a, int *b, Uint32 *c, Uint32 *d, Uint32 *e, Uint32 *f)
{
    void *addr = cosmo_dlsym(libD, "SDL_PixelFormatEnumToMasks");
    if (!IsWindows()) {
        SDL_bool (*local)(Uint32 a, int *b, Uint32 *c, Uint32 *d, Uint32 *e, Uint32 *f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(Uint32 a, int *b, Uint32 *c, Uint32 *d, Uint32 *e, Uint32 *f) = addr;
        return local(a, b, c, d, e, f);
    }
}
Uint32 abi_SDL_MasksToPixelFormatEnum(int a, Uint32 b, Uint32 c, Uint32 d, Uint32 e)
{
    void *addr = cosmo_dlsym(libD, "SDL_MasksToPixelFormatEnum");
    if (!IsWindows()) {
        Uint32 (*local)(int a, Uint32 b, Uint32 c, Uint32 d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(int a, Uint32 b, Uint32 c, Uint32 d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_PixelFormat *abi_SDL_AllocFormat(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AllocFormat");
    if (!IsWindows()) {
        SDL_PixelFormat *(*local)(Uint32 a) = addr;
        return local(a);
    } else {
        SDL_PixelFormat *__attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
void abi_SDL_FreeFormat(SDL_PixelFormat *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeFormat");
    if (!IsWindows()) {
        void (*local)(SDL_PixelFormat * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_PixelFormat * a) = addr;
        local(a);
    }
}
SDL_Palette *abi_SDL_AllocPalette(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AllocPalette");
    if (!IsWindows()) {
        SDL_Palette *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_Palette *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_SetPixelFormatPalette(SDL_PixelFormat *a, SDL_Palette *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetPixelFormatPalette");
    if (!IsWindows()) {
        int (*local)(SDL_PixelFormat * a, SDL_Palette * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_PixelFormat * a, SDL_Palette * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetPaletteColors(SDL_Palette *a, const SDL_Color *b, int c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetPaletteColors");
    if (!IsWindows()) {
        int (*local)(SDL_Palette * a, const SDL_Color *b, int c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Palette * a, const SDL_Color *b, int c, int d) = addr;
        return local(a, b, c, d);
    }
}
void abi_SDL_FreePalette(SDL_Palette *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreePalette");
    if (!IsWindows()) {
        void (*local)(SDL_Palette * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Palette * a) = addr;
        local(a);
    }
}
Uint32 abi_SDL_MapRGB(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_MapRGB");
    if (!IsWindows()) {
        Uint32 (*local)(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    }
}
Uint32 abi_SDL_MapRGBA(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d, Uint8 e)
{
    void *addr = cosmo_dlsym(libD, "SDL_MapRGBA");
    if (!IsWindows()) {
        Uint32 (*local)(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d, Uint8 e) = addr;
        return local(a, b, c, d, e);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(const SDL_PixelFormat *a, Uint8 b, Uint8 c, Uint8 d, Uint8 e) = addr;
        return local(a, b, c, d, e);
    }
}
void abi_SDL_GetRGB(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRGB");
    if (!IsWindows()) {
        void (*local)(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e) = addr;
        local(a, b, c, d, e);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e) = addr;
        local(a, b, c, d, e);
    }
}
void abi_SDL_GetRGBA(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e, Uint8 *f)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRGBA");
    if (!IsWindows()) {
        void (*local)(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e, Uint8 *f) = addr;
        local(a, b, c, d, e, f);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a, const SDL_PixelFormat *b, Uint8 *c, Uint8 *d, Uint8 *e, Uint8 *f) = addr;
        local(a, b, c, d, e, f);
    }
}
void abi_SDL_CalculateGammaRamp(float a, Uint16 *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_CalculateGammaRamp");
    if (!IsWindows()) {
        void (*local)(float a, Uint16 *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(float a, Uint16 *b) = addr;
        local(a, b);
    }
}
const char *abi_SDL_GetPlatform(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPlatform");
    if (!IsWindows()) {
        const char *(*local)(void) = addr;
        return local();
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_PowerState abi_SDL_GetPowerInfo(int *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPowerInfo");
    if (!IsWindows()) {
        SDL_PowerState (*local)(int *a, int *b) = addr;
        return local(a, b);
    } else {
        SDL_PowerState __attribute__((__ms_abi__)) (*local)(int *a, int *b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_HasIntersection(const SDL_Rect *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasIntersection");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_Rect *a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_IntersectRect(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_IntersectRect");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_UnionRect(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnionRect");
    if (!IsWindows()) {
        void (*local)(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *c) = addr;
        local(a, b, c);
    }
}
SDL_bool abi_SDL_EnclosePoints(const SDL_Point *a, int b, const SDL_Rect *c, SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_EnclosePoints");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_Point *a, int b, const SDL_Rect *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_Point *a, int b, const SDL_Rect *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
SDL_bool abi_SDL_IntersectRectAndLine(const SDL_Rect *a, int *b, int *c, int *d, int *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_IntersectRectAndLine");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_Rect *a, int *b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a, int *b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_GetNumRenderDrivers(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumRenderDrivers");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetRenderDriverInfo(int a, SDL_RendererInfo *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRenderDriverInfo");
    if (!IsWindows()) {
        int (*local)(int a, SDL_RendererInfo *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, SDL_RendererInfo *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_CreateWindowAndRenderer(int a, int b, Uint32 c, SDL_Window **d, SDL_Renderer **e)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateWindowAndRenderer");
    if (!IsWindows()) {
        int (*local)(int a, int b, Uint32 c, SDL_Window **d, SDL_Renderer **e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b, Uint32 c, SDL_Window **d, SDL_Renderer **e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_Renderer *abi_SDL_CreateRenderer(SDL_Window *a, int b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateRenderer");
    if (!IsWindows()) {
        SDL_Renderer *(*local)(SDL_Window * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        SDL_Renderer *__attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
SDL_Renderer *abi_SDL_CreateSoftwareRenderer(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateSoftwareRenderer");
    if (!IsWindows()) {
        SDL_Renderer *(*local)(SDL_Surface * a) = addr;
        return local(a);
    } else {
        SDL_Renderer *__attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        return local(a);
    }
}
SDL_Renderer *abi_SDL_GetRenderer(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRenderer");
    if (!IsWindows()) {
        SDL_Renderer *(*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_Renderer *__attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_GetRendererInfo(SDL_Renderer *a, SDL_RendererInfo *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRendererInfo");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_RendererInfo * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_RendererInfo * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetRendererOutputSize(SDL_Renderer *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRendererOutputSize");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, int *b, int *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int *b, int *c) = addr;
        return local(a, b, c);
    }
}
SDL_Texture *abi_SDL_CreateTexture(SDL_Renderer *a, Uint32 b, int c, int d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateTexture");
    if (!IsWindows()) {
        SDL_Texture *(*local)(SDL_Renderer * a, Uint32 b, int c, int d, int e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_Texture *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, Uint32 b, int c, int d, int e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_Texture *abi_SDL_CreateTextureFromSurface(SDL_Renderer *a, SDL_Surface *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateTextureFromSurface");
    if (!IsWindows()) {
        SDL_Texture *(*local)(SDL_Renderer * a, SDL_Surface * b) = addr;
        return local(a, b);
    } else {
        SDL_Texture *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Surface * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_QueryTexture(SDL_Texture *a, Uint32 *b, int *c, int *d, int *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_QueryTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, Uint32 * b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, Uint32 * b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_SetTextureColorMod(SDL_Texture *a, Uint8 b, Uint8 c, Uint8 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextureColorMod");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_GetTextureColorMod(SDL_Texture *a, Uint8 *b, Uint8 *c, Uint8 *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTextureColorMod");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, Uint8 * b, Uint8 * c, Uint8 * d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, Uint8 * b, Uint8 * c, Uint8 * d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_SetTextureAlphaMod(SDL_Texture *a, Uint8 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextureAlphaMod");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, Uint8 b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, Uint8 b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetTextureAlphaMod(SDL_Texture *a, Uint8 *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTextureAlphaMod");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, Uint8 * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, Uint8 * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetTextureBlendMode(SDL_Texture *a, SDL_BlendMode b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextureBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, SDL_BlendMode b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, SDL_BlendMode b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetTextureBlendMode(SDL_Texture *a, SDL_BlendMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTextureBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_UpdateTexture(SDL_Texture *a, const SDL_Rect *b, const void *c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpdateTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, const SDL_Rect *b, const void *c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, const SDL_Rect *b, const void *c, int d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_UpdateYUVTexture(SDL_Texture *a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f, const Uint8 *g, int h)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpdateYUVTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f, const Uint8 *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f, const Uint8 *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    }
}
int abi_SDL_LockTexture(SDL_Texture *a, const SDL_Rect *b, void **c, int *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, const SDL_Rect *b, void **c, int *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, const SDL_Rect *b, void **c, int *d) = addr;
        return local(a, b, c, d);
    }
}
void abi_SDL_UnlockTexture(SDL_Texture *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockTexture");
    if (!IsWindows()) {
        void (*local)(SDL_Texture * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Texture * a) = addr;
        local(a);
    }
}
SDL_bool abi_SDL_RenderTargetSupported(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderTargetSupported");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetRenderTarget(SDL_Renderer *a, SDL_Texture *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetRenderTarget");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b) = addr;
        return local(a, b);
    }
}
SDL_Texture *abi_SDL_GetRenderTarget(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRenderTarget");
    if (!IsWindows()) {
        SDL_Texture *(*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        SDL_Texture *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
int abi_SDL_RenderSetLogicalSize(SDL_Renderer *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetLogicalSize");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, int b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int b, int c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_RenderGetLogicalSize(SDL_Renderer *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetLogicalSize");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
int abi_SDL_RenderSetViewport(SDL_Renderer *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetViewport");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_RenderGetViewport(SDL_Renderer *a, SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetViewport");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, SDL_Rect * b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Rect * b) = addr;
        local(a, b);
    }
}
int abi_SDL_RenderSetClipRect(SDL_Renderer *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetClipRect");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_RenderGetClipRect(SDL_Renderer *a, SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetClipRect");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, SDL_Rect * b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Rect * b) = addr;
        local(a, b);
    }
}
int abi_SDL_RenderSetScale(SDL_Renderer *a, float b, float c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetScale");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, float b, float c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, float b, float c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_RenderGetScale(SDL_Renderer *a, float *b, float *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetScale");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, float *b, float *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, float *b, float *c) = addr;
        local(a, b, c);
    }
}
int abi_SDL_SetRenderDrawColor(SDL_Renderer *a, Uint8 b, Uint8 c, Uint8 d, Uint8 e)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetRenderDrawColor");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, Uint8 b, Uint8 c, Uint8 d, Uint8 e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, Uint8 b, Uint8 c, Uint8 d, Uint8 e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_GetRenderDrawColor(SDL_Renderer *a, Uint8 *b, Uint8 *c, Uint8 *d, Uint8 *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRenderDrawColor");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, Uint8 * b, Uint8 * c, Uint8 * d, Uint8 * e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, Uint8 * b, Uint8 * c, Uint8 * d, Uint8 * e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_SetRenderDrawBlendMode(SDL_Renderer *a, SDL_BlendMode b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetRenderDrawBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_BlendMode b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_BlendMode b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetRenderDrawBlendMode(SDL_Renderer *a, SDL_BlendMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRenderDrawBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderClear(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderClear");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
int abi_SDL_RenderDrawPoint(SDL_Renderer *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawPoint");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, int b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawPoints(SDL_Renderer *a, const SDL_Point *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawPoints");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Point *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Point *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawLine(SDL_Renderer *a, int b, int c, int d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawLine");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, int b, int c, int d, int e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int b, int c, int d, int e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_RenderDrawLines(SDL_Renderer *a, const SDL_Point *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawLines");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Point *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Point *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawRect(SDL_Renderer *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawRect");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderDrawRects(SDL_Renderer *a, const SDL_Rect *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawRects");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderFillRect(SDL_Renderer *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderFillRect");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderFillRects(SDL_Renderer *a, const SDL_Rect *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderFillRects");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderCopy(SDL_Renderer *a, SDL_Texture *b, const SDL_Rect *c, const SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderCopy");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_RenderCopyEx(SDL_Renderer *a, SDL_Texture *b, const SDL_Rect *c, const SDL_Rect *d, const double e, const SDL_Point *f, const SDL_RendererFlip g)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderCopyEx");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_Rect *d, const double e, const SDL_Point *f, const SDL_RendererFlip g) = addr;
        return local(a, b, c, d, e, f, g);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_Rect *d, const double e, const SDL_Point *f, const SDL_RendererFlip g) = addr;
        return local(a, b, c, d, e, f, g);
    }
}
int abi_SDL_RenderReadPixels(SDL_Renderer *a, const SDL_Rect *b, Uint32 c, void *d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderReadPixels");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_Rect *b, Uint32 c, void *d, int e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_Rect *b, Uint32 c, void *d, int e) = addr;
        return local(a, b, c, d, e);
    }
}
void abi_SDL_RenderPresent(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderPresent");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        local(a);
    }
}
void abi_SDL_DestroyTexture(SDL_Texture *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyTexture");
    if (!IsWindows()) {
        void (*local)(SDL_Texture * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Texture * a) = addr;
        local(a);
    }
}
void abi_SDL_DestroyRenderer(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyRenderer");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        local(a);
    }
}
int abi_SDL_GL_BindTexture(SDL_Texture *a, float *b, float *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_BindTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, float *b, float *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, float *b, float *c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GL_UnbindTexture(SDL_Texture *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_UnbindTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a) = addr;
        return local(a);
    }
}
SDL_RWops *abi_SDL_RWFromFile(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWFromFile");
    if (!IsWindows()) {
        SDL_RWops *(*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        SDL_RWops *__attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
SDL_RWops *abi_SDL_RWFromMem(void *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWFromMem");
    if (!IsWindows()) {
        SDL_RWops *(*local)(void *a, int b) = addr;
        return local(a, b);
    } else {
        SDL_RWops *__attribute__((__ms_abi__)) (*local)(void *a, int b) = addr;
        return local(a, b);
    }
}
SDL_RWops *abi_SDL_RWFromConstMem(const void *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWFromConstMem");
    if (!IsWindows()) {
        SDL_RWops *(*local)(const void *a, int b) = addr;
        return local(a, b);
    } else {
        SDL_RWops *__attribute__((__ms_abi__)) (*local)(const void *a, int b) = addr;
        return local(a, b);
    }
}
SDL_RWops *abi_SDL_AllocRW(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_AllocRW");
    if (!IsWindows()) {
        SDL_RWops *(*local)(void) = addr;
        return local();
    } else {
        SDL_RWops *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_FreeRW(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeRW");
    if (!IsWindows()) {
        void (*local)(SDL_RWops * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        local(a);
    }
}
Uint8 abi_SDL_ReadU8(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadU8");
    if (!IsWindows()) {
        Uint8 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint8 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_ReadLE16(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadLE16");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_ReadBE16(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadBE16");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_ReadLE32(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadLE32");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_ReadBE32(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadBE32");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint64 abi_SDL_ReadLE64(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadLE64");
    if (!IsWindows()) {
        Uint64 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Uint64 abi_SDL_ReadBE64(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ReadBE64");
    if (!IsWindows()) {
        Uint64 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
size_t abi_SDL_WriteU8(SDL_RWops *a, Uint8 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteU8");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint8 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint8 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteLE16(SDL_RWops *a, Uint16 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteLE16");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint16 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint16 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteBE16(SDL_RWops *a, Uint16 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteBE16");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint16 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint16 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteLE32(SDL_RWops *a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteLE32");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint32 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint32 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteBE32(SDL_RWops *a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteBE32");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint32 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint32 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteLE64(SDL_RWops *a, Uint64 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteLE64");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint64 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint64 b) = addr;
        return local(a, b);
    }
}
size_t abi_SDL_WriteBE64(SDL_RWops *a, Uint64 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WriteBE64");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, Uint64 b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Uint64 b) = addr;
        return local(a, b);
    }
}
SDL_Window *abi_SDL_CreateShapedWindow(const char *a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, Uint32 f)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateShapedWindow");
    if (!IsWindows()) {
        SDL_Window *(*local)(const char *a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(const char *a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    }
}
SDL_bool abi_SDL_IsShapedWindow(const SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsShapedWindow");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_Window *a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_Window *a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowShape(SDL_Window *a, SDL_Surface *b, SDL_WindowShapeMode *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowShape");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_Surface * b, SDL_WindowShapeMode * c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_Surface * b, SDL_WindowShapeMode * c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GetShapedWindowMode(SDL_Window *a, SDL_WindowShapeMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetShapedWindowMode");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_WindowShapeMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_WindowShapeMode * b) = addr;
        return local(a, b);
    }
}
void *abi_SDL_malloc(size_t a)
{
    void *addr = cosmo_dlsym(libD, "SDL_malloc");
    if (!IsWindows()) {
        void *(*local)(size_t a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(size_t a) = addr;
        return local(a);
    }
}
void *abi_SDL_calloc(size_t a, size_t b)
{
    void *addr = cosmo_dlsym(libD, "SDL_calloc");
    if (!IsWindows()) {
        void *(*local)(size_t a, size_t b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(size_t a, size_t b) = addr;
        return local(a, b);
    }
}
void *abi_SDL_realloc(void *a, size_t b)
{
    void *addr = cosmo_dlsym(libD, "SDL_realloc");
    if (!IsWindows()) {
        void *(*local)(void *a, size_t b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(void *a, size_t b) = addr;
        return local(a, b);
    }
}
void abi_SDL_free(void *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_free");
    if (!IsWindows()) {
        void (*local)(void *a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(void *a) = addr;
        local(a);
    }
}
char *abi_SDL_getenv(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_getenv");
    if (!IsWindows()) {
        char *(*local)(const char *a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
int abi_SDL_setenv(const char *a, const char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_setenv");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b, int c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_qsort(void *a, size_t b, size_t c, int(SDLCALL *d)(const void *, const void *))
{
    void *addr = cosmo_dlsym(libD, "SDL_qsort");
    if (!IsWindows()) {
        void (*local)(void *a, size_t b, size_t c, int(SDLCALL * d)(const void *, const void *)) = addr;
        local(a, b, c, d);
    } else {
        void __attribute__((__ms_abi__)) (*local)(void *a, size_t b, size_t c, int(SDLCALL * d)(const void *, const void *)) = addr;
        local(a, b, c, d);
    }
}
int abi_SDL_abs(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_abs");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isdigit(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isdigit");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isspace(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isspace");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_toupper(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_toupper");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_tolower(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_tolower");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void *abi_SDL_memset(SDL_OUT_BYTECAP(c) void *a, int b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_memset");
    if (!IsWindows()) {
        void *(*local)(SDL_OUT_BYTECAP(c) void *a, int b, size_t c) = addr;
        return local(a, b, c);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_OUT_BYTECAP(c) void *a, int b, size_t c) = addr;
        return local(a, b, c);
    }
}
void *abi_SDL_memcpy(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_memcpy");
    if (!IsWindows()) {
        void *(*local)(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c) = addr;
        return local(a, b, c);
    }
}
void *abi_SDL_memmove(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_memmove");
    if (!IsWindows()) {
        void *(*local)(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_OUT_BYTECAP(c) void *a, SDL_IN_BYTECAP(c) const void *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_memcmp(const void *a, const void *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_memcmp");
    if (!IsWindows()) {
        int (*local)(const void *a, const void *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const void *a, const void *b, size_t c) = addr;
        return local(a, b, c);
    }
}
size_t abi_SDL_wcslen(const wchar_t *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcslen");
    if (!IsWindows()) {
        size_t (*local)(const wchar_t *a) = addr;
        return local(a);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(const wchar_t *a) = addr;
        return local(a);
    }
}
size_t abi_SDL_wcslcpy(SDL_OUT_Z_CAP(c) wchar_t *a, const wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcslcpy");
    if (!IsWindows()) {
        size_t (*local)(SDL_OUT_Z_CAP(c) wchar_t * a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_OUT_Z_CAP(c) wchar_t * a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    }
}
size_t abi_SDL_wcslcat(SDL_INOUT_Z_CAP(c) wchar_t *a, const wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcslcat");
    if (!IsWindows()) {
        size_t (*local)(SDL_INOUT_Z_CAP(c) wchar_t * a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_INOUT_Z_CAP(c) wchar_t * a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    }
}
size_t abi_SDL_strlen(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_strlen");
    if (!IsWindows()) {
        size_t (*local)(const char *a) = addr;
        return local(a);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
size_t abi_SDL_strlcpy(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strlcpy");
    if (!IsWindows()) {
        size_t (*local)(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
size_t abi_SDL_utf8strlcpy(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_utf8strlcpy");
    if (!IsWindows()) {
        size_t (*local)(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_OUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
size_t abi_SDL_strlcat(SDL_INOUT_Z_CAP(c) char *a, const char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strlcat");
    if (!IsWindows()) {
        size_t (*local)(SDL_INOUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_INOUT_Z_CAP(c) char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_strdup(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_strdup");
    if (!IsWindows()) {
        char *(*local)(const char *a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_strrev(char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_strrev");
    if (!IsWindows()) {
        char *(*local)(char *a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_strupr(char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_strupr");
    if (!IsWindows()) {
        char *(*local)(char *a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_strlwr(char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_strlwr");
    if (!IsWindows()) {
        char *(*local)(char *a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_strchr(const char *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strchr");
    if (!IsWindows()) {
        char *(*local)(const char *a, int b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, int b) = addr;
        return local(a, b);
    }
}
char *abi_SDL_strrchr(const char *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strrchr");
    if (!IsWindows()) {
        char *(*local)(const char *a, int b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, int b) = addr;
        return local(a, b);
    }
}
char *abi_SDL_strstr(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strstr");
    if (!IsWindows()) {
        char *(*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
char *abi_SDL_itoa(int a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_itoa");
    if (!IsWindows()) {
        char *(*local)(int a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(int a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_uitoa(unsigned int a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_uitoa");
    if (!IsWindows()) {
        char *(*local)(unsigned int a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(unsigned int a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_ltoa(long a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_ltoa");
    if (!IsWindows()) {
        char *(*local)(long a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(long a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_ultoa(unsigned long a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_ultoa");
    if (!IsWindows()) {
        char *(*local)(unsigned long a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(unsigned long a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_lltoa(Sint64 a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_lltoa");
    if (!IsWindows()) {
        char *(*local)(Sint64 a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(Sint64 a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_ulltoa(Uint64 a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_ulltoa");
    if (!IsWindows()) {
        char *(*local)(Uint64 a, char *b, int c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(Uint64 a, char *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_atoi(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_atoi");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
double abi_SDL_atof(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_atof");
    if (!IsWindows()) {
        double (*local)(const char *a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
long abi_SDL_strtol(const char *a, char **b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtol");
    if (!IsWindows()) {
        long (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    } else {
        long __attribute__((__ms_abi__)) (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    }
}
unsigned long abi_SDL_strtoul(const char *a, char **b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtoul");
    if (!IsWindows()) {
        unsigned long (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    } else {
        unsigned long __attribute__((__ms_abi__)) (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    }
}
Sint64 abi_SDL_strtoll(const char *a, char **b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtoll");
    if (!IsWindows()) {
        Sint64 (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    } else {
        Sint64 __attribute__((__ms_abi__)) (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    }
}
Uint64 abi_SDL_strtoull(const char *a, char **b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtoull");
    if (!IsWindows()) {
        Uint64 (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(const char *a, char **b, int c) = addr;
        return local(a, b, c);
    }
}
double abi_SDL_strtod(const char *a, char **b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtod");
    if (!IsWindows()) {
        double (*local)(const char *a, char **b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(const char *a, char **b) = addr;
        return local(a, b);
    }
}
int abi_SDL_strcmp(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strcmp");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_strncmp(const char *a, const char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strncmp");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_strcasecmp(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strcasecmp");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_strncasecmp(const char *a, const char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strncasecmp");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_vsnprintf(SDL_OUT_Z_CAP(b) char *a, size_t b, const char *c, va_list d)
{
    void *addr = cosmo_dlsym(libD, "SDL_vsnprintf");
    if (!IsWindows()) {
        int (*local)(SDL_OUT_Z_CAP(b) char *a, size_t b, const char *c, va_list d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_OUT_Z_CAP(b) char *a, size_t b, const char *c, va_list d) = addr;
        return local(a, b, c, d);
    }
}
double abi_SDL_acos(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_acos");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_asin(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_asin");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_atan(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_atan");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_atan2(double a, double b)
{
    void *addr = cosmo_dlsym(libD, "SDL_atan2");
    if (!IsWindows()) {
        double (*local)(double a, double b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a, double b) = addr;
        return local(a, b);
    }
}
double abi_SDL_ceil(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ceil");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_copysign(double a, double b)
{
    void *addr = cosmo_dlsym(libD, "SDL_copysign");
    if (!IsWindows()) {
        double (*local)(double a, double b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a, double b) = addr;
        return local(a, b);
    }
}
double abi_SDL_cos(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_cos");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_cosf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_cosf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
double abi_SDL_fabs(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_fabs");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_floor(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_floor");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_log(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_log");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
double abi_SDL_pow(double a, double b)
{
    void *addr = cosmo_dlsym(libD, "SDL_pow");
    if (!IsWindows()) {
        double (*local)(double a, double b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a, double b) = addr;
        return local(a, b);
    }
}
double abi_SDL_scalbn(double a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_scalbn");
    if (!IsWindows()) {
        double (*local)(double a, int b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a, int b) = addr;
        return local(a, b);
    }
}
double abi_SDL_sin(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_sin");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_sinf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_sinf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
double abi_SDL_sqrt(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_sqrt");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
SDL_iconv_t abi_SDL_iconv_open(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_iconv_open");
    if (!IsWindows()) {
        SDL_iconv_t (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        SDL_iconv_t __attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_iconv_close(SDL_iconv_t a)
{
    void *addr = cosmo_dlsym(libD, "SDL_iconv_close");
    if (!IsWindows()) {
        int (*local)(SDL_iconv_t a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_iconv_t a) = addr;
        return local(a);
    }
}
size_t abi_SDL_iconv(SDL_iconv_t a, const char **b, size_t *c, char **d, size_t *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_iconv");
    if (!IsWindows()) {
        size_t (*local)(SDL_iconv_t a, const char **b, size_t *c, char **d, size_t *e) = addr;
        return local(a, b, c, d, e);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_iconv_t a, const char **b, size_t *c, char **d, size_t *e) = addr;
        return local(a, b, c, d, e);
    }
}
char *abi_SDL_iconv_string(const char *a, const char *b, const char *c, size_t d)
{
    void *addr = cosmo_dlsym(libD, "SDL_iconv_string");
    if (!IsWindows()) {
        char *(*local)(const char *a, const char *b, const char *c, size_t d) = addr;
        return local(a, b, c, d);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, const char *b, const char *c, size_t d) = addr;
        return local(a, b, c, d);
    }
}
SDL_Surface *abi_SDL_CreateRGBSurface(Uint32 a, int b, int c, int d, Uint32 e, Uint32 f, Uint32 g, Uint32 h)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateRGBSurface");
    if (!IsWindows()) {
        SDL_Surface *(*local)(Uint32 a, int b, int c, int d, Uint32 e, Uint32 f, Uint32 g, Uint32 h) = addr;
        return local(a, b, c, d, e, f, g, h);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(Uint32 a, int b, int c, int d, Uint32 e, Uint32 f, Uint32 g, Uint32 h) = addr;
        return local(a, b, c, d, e, f, g, h);
    }
}
SDL_Surface *abi_SDL_CreateRGBSurfaceFrom(void *a, int b, int c, int d, int e, Uint32 f, Uint32 g, Uint32 h, Uint32 i)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateRGBSurfaceFrom");
    if (!IsWindows()) {
        SDL_Surface *(*local)(void *a, int b, int c, int d, int e, Uint32 f, Uint32 g, Uint32 h, Uint32 i) = addr;
        return local(a, b, c, d, e, f, g, h, i);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(void *a, int b, int c, int d, int e, Uint32 f, Uint32 g, Uint32 h, Uint32 i) = addr;
        return local(a, b, c, d, e, f, g, h, i);
    }
}
void abi_SDL_FreeSurface(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeSurface");
    if (!IsWindows()) {
        void (*local)(SDL_Surface * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        local(a);
    }
}
int abi_SDL_SetSurfacePalette(SDL_Surface *a, SDL_Palette *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetSurfacePalette");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_Palette * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_Palette * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_LockSurface(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockSurface");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        return local(a);
    }
}
void abi_SDL_UnlockSurface(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockSurface");
    if (!IsWindows()) {
        void (*local)(SDL_Surface * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        local(a);
    }
}
SDL_Surface *abi_SDL_LoadBMP_RW(SDL_RWops *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadBMP_RW");
    if (!IsWindows()) {
        SDL_Surface *(*local)(SDL_RWops * a, int b) = addr;
        return local(a, b);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(SDL_RWops * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SaveBMP_RW(SDL_Surface *a, SDL_RWops *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SaveBMP_RW");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_RWops * b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_RWops * b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_SetSurfaceRLE(SDL_Surface *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetSurfaceRLE");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetColorKey(SDL_Surface *a, int b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetColorKey");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, int b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GetColorKey(SDL_Surface *a, Uint32 *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetColorKey");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, Uint32 * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint32 * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetSurfaceColorMod(SDL_Surface *a, Uint8 b, Uint8 c, Uint8 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetSurfaceColorMod");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_GetSurfaceColorMod(SDL_Surface *a, Uint8 *b, Uint8 *c, Uint8 *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetSurfaceColorMod");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, Uint8 * b, Uint8 * c, Uint8 * d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint8 * b, Uint8 * c, Uint8 * d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_SetSurfaceAlphaMod(SDL_Surface *a, Uint8 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetSurfaceAlphaMod");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, Uint8 b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint8 b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetSurfaceAlphaMod(SDL_Surface *a, Uint8 *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetSurfaceAlphaMod");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, Uint8 * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint8 * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetSurfaceBlendMode(SDL_Surface *a, SDL_BlendMode b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetSurfaceBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_BlendMode b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_BlendMode b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetSurfaceBlendMode(SDL_Surface *a, SDL_BlendMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetSurfaceBlendMode");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_BlendMode * b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_SetClipRect(SDL_Surface *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetClipRect");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Surface * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_GetClipRect(SDL_Surface *a, SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetClipRect");
    if (!IsWindows()) {
        void (*local)(SDL_Surface * a, SDL_Rect * b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_Rect * b) = addr;
        local(a, b);
    }
}
SDL_Surface *abi_SDL_ConvertSurface(SDL_Surface *a, const SDL_PixelFormat *b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_ConvertSurface");
    if (!IsWindows()) {
        SDL_Surface *(*local)(SDL_Surface * a, const SDL_PixelFormat *b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_PixelFormat *b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
SDL_Surface *abi_SDL_ConvertSurfaceFormat(SDL_Surface *a, Uint32 b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_ConvertSurfaceFormat");
    if (!IsWindows()) {
        SDL_Surface *(*local)(SDL_Surface * a, Uint32 b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(SDL_Surface * a, Uint32 b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_ConvertPixels(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h)
{
    void *addr = cosmo_dlsym(libD, "SDL_ConvertPixels");
    if (!IsWindows()) {
        int (*local)(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    }
}
int abi_SDL_FillRect(SDL_Surface *a, const SDL_Rect *b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_FillRect");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_FillRects(SDL_Surface *a, const SDL_Rect *b, int c, Uint32 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_FillRects");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, int c, Uint32 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, int c, Uint32 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_UpperBlit(SDL_Surface *a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpperBlit");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_LowerBlit(SDL_Surface *a, SDL_Rect *b, SDL_Surface *c, SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_LowerBlit");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_Rect * b, SDL_Surface * c, SDL_Rect * d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_Rect * b, SDL_Surface * c, SDL_Rect * d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_SoftStretch(SDL_Surface *a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SoftStretch");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_UpperBlitScaled(SDL_Surface *a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpperBlitScaled");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_LowerBlitScaled(SDL_Surface *a, SDL_Rect *b, SDL_Surface *c, SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_LowerBlitScaled");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, SDL_Rect * b, SDL_Surface * c, SDL_Rect * d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, SDL_Rect * b, SDL_Surface * c, SDL_Rect * d) = addr;
        return local(a, b, c, d);
    }
}
const char *abi_SDL_GetThreadName(SDL_Thread *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetThreadName");
    if (!IsWindows()) {
        const char *(*local)(SDL_Thread * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Thread * a) = addr;
        return local(a);
    }
}
SDL_threadID abi_SDL_ThreadID(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ThreadID");
    if (!IsWindows()) {
        SDL_threadID (*local)(void) = addr;
        return local();
    } else {
        SDL_threadID __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_threadID abi_SDL_GetThreadID(SDL_Thread *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetThreadID");
    if (!IsWindows()) {
        SDL_threadID (*local)(SDL_Thread * a) = addr;
        return local(a);
    } else {
        SDL_threadID __attribute__((__ms_abi__)) (*local)(SDL_Thread * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetThreadPriority(SDL_ThreadPriority a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetThreadPriority");
    if (!IsWindows()) {
        int (*local)(SDL_ThreadPriority a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_ThreadPriority a) = addr;
        return local(a);
    }
}
void abi_SDL_WaitThread(SDL_Thread *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WaitThread");
    if (!IsWindows()) {
        void (*local)(SDL_Thread * a, int *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Thread * a, int *b) = addr;
        local(a, b);
    }
}
void abi_SDL_DetachThread(SDL_Thread *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DetachThread");
    if (!IsWindows()) {
        void (*local)(SDL_Thread * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Thread * a) = addr;
        local(a);
    }
}
SDL_TLSID abi_SDL_TLSCreate(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_TLSCreate");
    if (!IsWindows()) {
        SDL_TLSID (*local)
        (void) = addr;
        return local();
    } else {
        SDL_TLSID __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void *abi_SDL_TLSGet(SDL_TLSID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_TLSGet");
    if (!IsWindows()) {
        void *(*local)(SDL_TLSID a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_TLSID a) = addr;
        return local(a);
    }
}
int abi_SDL_TLSSet(SDL_TLSID a, const void *b, void(SDLCALL *c)(void *))
{
    void *addr = cosmo_dlsym(libD, "SDL_TLSSet");
    if (!IsWindows()) {
        int (*local)(SDL_TLSID a, const void *b, void(SDLCALL * c)(void *)) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_TLSID a, const void *b, void(SDLCALL * c)(void *)) = addr;
        return local(a, b, c);
    }
}
Uint32 abi_SDL_GetTicks(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTicks");
    if (!IsWindows()) {
        Uint32 (*local)(void) = addr;
        return local();
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
Uint64 abi_SDL_GetPerformanceCounter(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPerformanceCounter");
    if (!IsWindows()) {
        Uint64 (*local)(void) = addr;
        return local();
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
Uint64 abi_SDL_GetPerformanceFrequency(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPerformanceFrequency");
    if (!IsWindows()) {
        Uint64 (*local)(void) = addr;
        return local();
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_Delay(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Delay");
    if (!IsWindows()) {
        void (*local)(Uint32 a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        local(a);
    }
}
SDL_TimerID abi_SDL_AddTimer(Uint32 a, SDL_TimerCallback b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AddTimer");
    if (!IsWindows()) {
        SDL_TimerID (*local)(Uint32 a, SDL_TimerCallback b, void *c) = addr;
        return local(a, b, c);
    } else {
        SDL_TimerID __attribute__((__ms_abi__)) (*local)(Uint32 a, SDL_TimerCallback b, void *c) = addr;
        return local(a, b, c);
    }
}
SDL_bool abi_SDL_RemoveTimer(SDL_TimerID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RemoveTimer");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_TimerID a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_TimerID a) = addr;
        return local(a);
    }
}
int abi_SDL_GetNumTouchDevices(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumTouchDevices");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_TouchID abi_SDL_GetTouchDevice(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTouchDevice");
    if (!IsWindows()) {
        SDL_TouchID (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_TouchID __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_GetNumTouchFingers(SDL_TouchID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumTouchFingers");
    if (!IsWindows()) {
        int (*local)(SDL_TouchID a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_TouchID a) = addr;
        return local(a);
    }
}
SDL_Finger *abi_SDL_GetTouchFinger(SDL_TouchID a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTouchFinger");
    if (!IsWindows()) {
        SDL_Finger *(*local)(SDL_TouchID a, int b) = addr;
        return local(a, b);
    } else {
        SDL_Finger *__attribute__((__ms_abi__)) (*local)(SDL_TouchID a, int b) = addr;
        return local(a, b);
    }
}
void abi_SDL_GetVersion(SDL_version *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetVersion");
    if (!IsWindows()) {
        void (*local)(SDL_version * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_version * a) = addr;
        local(a);
    }
}
const char *abi_SDL_GetRevision(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRevision");
    if (!IsWindows()) {
        const char *(*local)(void) = addr;
        return local();
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetRevisionNumber(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRevisionNumber");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetNumVideoDrivers(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumVideoDrivers");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_GetVideoDriver(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetVideoDriver");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_VideoInit(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_VideoInit");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void abi_SDL_VideoQuit(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_VideoQuit");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
const char *abi_SDL_GetCurrentVideoDriver(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCurrentVideoDriver");
    if (!IsWindows()) {
        const char *(*local)(void) = addr;
        return local();
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetNumVideoDisplays(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumVideoDisplays");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_GetDisplayName(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayName");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_GetDisplayBounds(int a, SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayBounds");
    if (!IsWindows()) {
        int (*local)(int a, SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, SDL_Rect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetNumDisplayModes(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumDisplayModes");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_GetDisplayMode(int a, int b, SDL_DisplayMode *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayMode");
    if (!IsWindows()) {
        int (*local)(int a, int b, SDL_DisplayMode *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b, SDL_DisplayMode *c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GetDesktopDisplayMode(int a, SDL_DisplayMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDesktopDisplayMode");
    if (!IsWindows()) {
        int (*local)(int a, SDL_DisplayMode *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, SDL_DisplayMode *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetCurrentDisplayMode(int a, SDL_DisplayMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetCurrentDisplayMode");
    if (!IsWindows()) {
        int (*local)(int a, SDL_DisplayMode *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, SDL_DisplayMode *b) = addr;
        return local(a, b);
    }
}
SDL_DisplayMode *abi_SDL_GetClosestDisplayMode(int a, const SDL_DisplayMode *b, SDL_DisplayMode *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetClosestDisplayMode");
    if (!IsWindows()) {
        SDL_DisplayMode *(*local)(int a, const SDL_DisplayMode *b, SDL_DisplayMode *c) = addr;
        return local(a, b, c);
    } else {
        SDL_DisplayMode *__attribute__((__ms_abi__)) (*local)(int a, const SDL_DisplayMode *b, SDL_DisplayMode *c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GetWindowDisplayIndex(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowDisplayIndex");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowDisplayMode(SDL_Window *a, const SDL_DisplayMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowDisplayMode");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, const SDL_DisplayMode *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, const SDL_DisplayMode *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetWindowDisplayMode(SDL_Window *a, SDL_DisplayMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowDisplayMode");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_DisplayMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_DisplayMode * b) = addr;
        return local(a, b);
    }
}
Uint32 abi_SDL_GetWindowPixelFormat(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowPixelFormat");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
SDL_Window *abi_SDL_CreateWindow(const char *a, int b, int c, int d, int e, Uint32 f)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateWindow");
    if (!IsWindows()) {
        SDL_Window *(*local)(const char *a, int b, int c, int d, int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(const char *a, int b, int c, int d, int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    }
}
SDL_Window *abi_SDL_CreateWindowFrom(const void *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateWindowFrom");
    if (!IsWindows()) {
        SDL_Window *(*local)(const void *a) = addr;
        return local(a);
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(const void *a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_GetWindowID(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowID");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
SDL_Window *abi_SDL_GetWindowFromID(Uint32 a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowFromID");
    if (!IsWindows()) {
        SDL_Window *(*local)(Uint32 a) = addr;
        return local(a);
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(Uint32 a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_GetWindowFlags(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowFlags");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
void abi_SDL_SetWindowTitle(SDL_Window *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowTitle");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, const char *b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, const char *b) = addr;
        local(a, b);
    }
}
const char *abi_SDL_GetWindowTitle(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowTitle");
    if (!IsWindows()) {
        const char *(*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
void abi_SDL_SetWindowIcon(SDL_Window *a, SDL_Surface *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowIcon");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_Surface * b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_Surface * b) = addr;
        local(a, b);
    }
}
void *abi_SDL_SetWindowData(SDL_Window *a, const char *b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowData");
    if (!IsWindows()) {
        void *(*local)(SDL_Window * a, const char *b, void *c) = addr;
        return local(a, b, c);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Window * a, const char *b, void *c) = addr;
        return local(a, b, c);
    }
}
void *abi_SDL_GetWindowData(SDL_Window *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowData");
    if (!IsWindows()) {
        void *(*local)(SDL_Window * a, const char *b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Window * a, const char *b) = addr;
        return local(a, b);
    }
}
void abi_SDL_SetWindowPosition(SDL_Window *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowPosition");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_GetWindowPosition(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowPosition");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_SetWindowSize(SDL_Window *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_GetWindowSize(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_SetWindowMinimumSize(SDL_Window *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowMinimumSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_GetWindowMinimumSize(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowMinimumSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_SetWindowMaximumSize(SDL_Window *a, int b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowMaximumSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int b, int c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_GetWindowMaximumSize(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowMaximumSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_SetWindowBordered(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowBordered");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
void abi_SDL_ShowWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ShowWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_HideWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HideWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_RaiseWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RaiseWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_MaximizeWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_MaximizeWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_MinimizeWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_MinimizeWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_RestoreWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RestoreWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
int abi_SDL_SetWindowFullscreen(SDL_Window *a, Uint32 b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowFullscreen");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, Uint32 b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, Uint32 b) = addr;
        return local(a, b);
    }
}
SDL_Surface *abi_SDL_GetWindowSurface(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowSurface");
    if (!IsWindows()) {
        SDL_Surface *(*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_UpdateWindowSurface(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpdateWindowSurface");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_UpdateWindowSurfaceRects(SDL_Window *a, const SDL_Rect *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpdateWindowSurfaceRects");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, const SDL_Rect *b, int c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_SetWindowGrab(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowGrab");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
SDL_bool abi_SDL_GetWindowGrab(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowGrab");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowBrightness(SDL_Window *a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowBrightness");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, float b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, float b) = addr;
        return local(a, b);
    }
}
float abi_SDL_GetWindowBrightness(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowBrightness");
    if (!IsWindows()) {
        float (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowGammaRamp(SDL_Window *a, const Uint16 *b, const Uint16 *c, const Uint16 *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowGammaRamp");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, const Uint16 *b, const Uint16 *c, const Uint16 *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, const Uint16 *b, const Uint16 *c, const Uint16 *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_GetWindowGammaRamp(SDL_Window *a, Uint16 *b, Uint16 *c, Uint16 *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowGammaRamp");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, Uint16 * b, Uint16 * c, Uint16 * d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, Uint16 * b, Uint16 * c, Uint16 * d) = addr;
        return local(a, b, c, d);
    }
}
void abi_SDL_DestroyWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
SDL_bool abi_SDL_IsScreenSaverEnabled(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsScreenSaverEnabled");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_EnableScreenSaver(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_EnableScreenSaver");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_DisableScreenSaver(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_DisableScreenSaver");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_GL_LoadLibrary(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_LoadLibrary");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void *abi_SDL_GL_GetProcAddress(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetProcAddress");
    if (!IsWindows()) {
        void *(*local)(const char *a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void abi_SDL_GL_UnloadLibrary(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_UnloadLibrary");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_GL_ExtensionSupported(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_ExtensionSupported");
    if (!IsWindows()) {
        SDL_bool (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
int abi_SDL_GL_SetAttribute(SDL_GLattr a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_SetAttribute");
    if (!IsWindows()) {
        int (*local)(SDL_GLattr a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GLattr a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GL_GetAttribute(SDL_GLattr a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetAttribute");
    if (!IsWindows()) {
        int (*local)(SDL_GLattr a, int *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GLattr a, int *b) = addr;
        return local(a, b);
    }
}
SDL_GLContext abi_SDL_GL_CreateContext(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_CreateContext");
    if (!IsWindows()) {
        SDL_GLContext (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_GLContext __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_GL_MakeCurrent(SDL_Window *a, SDL_GLContext b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_MakeCurrent");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_GLContext b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_GLContext b) = addr;
        return local(a, b);
    }
}
SDL_Window *abi_SDL_GL_GetCurrentWindow(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetCurrentWindow");
    if (!IsWindows()) {
        SDL_Window *(*local)(void) = addr;
        return local();
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_GLContext abi_SDL_GL_GetCurrentContext(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetCurrentContext");
    if (!IsWindows()) {
        SDL_GLContext (*local)(void) = addr;
        return local();
    } else {
        SDL_GLContext __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_GL_GetDrawableSize(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetDrawableSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
int abi_SDL_GL_SetSwapInterval(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_SetSwapInterval");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_GL_GetSwapInterval(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_GetSwapInterval");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void abi_SDL_GL_SwapWindow(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_SwapWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        local(a);
    }
}
void abi_SDL_GL_DeleteContext(SDL_GLContext a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_DeleteContext");
    if (!IsWindows()) {
        void (*local)(SDL_GLContext a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_GLContext a) = addr;
        local(a);
    }
}
int abi_SDL_vsscanf(const char *a, const char *b, va_list c)
{
    void *addr = cosmo_dlsym(libD, "SDL_vsscanf");
    if (!IsWindows()) {
        int (*local)(const char *a, const char *b, va_list c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a, const char *b, va_list c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GameControllerAddMappingsFromRW(SDL_RWops *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerAddMappingsFromRW");
    if (!IsWindows()) {
        int (*local)(SDL_RWops * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, int b) = addr;
        return local(a, b);
    }
}
void abi_SDL_GL_ResetAttributes(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GL_ResetAttributes");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_HasAVX(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasAVX");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_AssertionHandler abi_SDL_GetDefaultAssertionHandler(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDefaultAssertionHandler");
    if (!IsWindows()) {
        SDL_AssertionHandler (*local)(void) = addr;
        return local();
    } else {
        SDL_AssertionHandler __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_AssertionHandler abi_SDL_GetAssertionHandler(void **a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAssertionHandler");
    if (!IsWindows()) {
        SDL_AssertionHandler (*local)(void **a) = addr;
        return local(a);
    } else {
        SDL_AssertionHandler __attribute__((__ms_abi__)) (*local)(void **a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_RenderIsClipEnabled(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderIsClipEnabled");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
int abi_SDL_WarpMouseGlobal(int a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_WarpMouseGlobal");
    if (!IsWindows()) {
        int (*local)(int a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b) = addr;
        return local(a, b);
    }
}
float abi_SDL_sqrtf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_sqrtf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
double abi_SDL_tan(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_tan");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_tanf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_tanf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
int abi_SDL_CaptureMouse(SDL_bool a)
{
    void *addr = cosmo_dlsym(libD, "SDL_CaptureMouse");
    if (!IsWindows()) {
        int (*local)(SDL_bool a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_bool a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowHitTest(SDL_Window *a, SDL_HitTest b, void *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowHitTest");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_HitTest b, void *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_HitTest b, void *c) = addr;
        return local(a, b, c);
    }
}
Uint32 abi_SDL_GetGlobalMouseState(int *a, int *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetGlobalMouseState");
    if (!IsWindows()) {
        Uint32 (*local)(int *a, int *b) = addr;
        return local(a, b);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(int *a, int *b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_HasAVX2(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasAVX2");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_QueueAudio(SDL_AudioDeviceID a, const void *b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_QueueAudio");
    if (!IsWindows()) {
        int (*local)(SDL_AudioDeviceID a, const void *b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a, const void *b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
Uint32 abi_SDL_GetQueuedAudioSize(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetQueuedAudioSize");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_AudioDeviceID a) = addr;
        return local(a);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        return local(a);
    }
}
void abi_SDL_ClearQueuedAudio(SDL_AudioDeviceID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ClearQueuedAudio");
    if (!IsWindows()) {
        void (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a) = addr;
        local(a);
    }
}
SDL_Window *abi_SDL_GetGrabbedWindow(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetGrabbedWindow");
    if (!IsWindows()) {
        SDL_Window *(*local)(void) = addr;
        return local();
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GetDisplayDPI(int a, float *b, float *c, float *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayDPI");
    if (!IsWindows()) {
        int (*local)(int a, float *b, float *c, float *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, float *b, float *c, float *d) = addr;
        return local(a, b, c, d);
    }
}
SDL_JoystickPowerLevel abi_SDL_JoystickCurrentPowerLevel(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickCurrentPowerLevel");
    if (!IsWindows()) {
        SDL_JoystickPowerLevel (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_JoystickPowerLevel __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_GameController *abi_SDL_GameControllerFromInstanceID(SDL_JoystickID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerFromInstanceID");
    if (!IsWindows()) {
        SDL_GameController *(*local)(SDL_JoystickID a) = addr;
        return local(a);
    } else {
        SDL_GameController *__attribute__((__ms_abi__)) (*local)(SDL_JoystickID a) = addr;
        return local(a);
    }
}
SDL_Joystick *abi_SDL_JoystickFromInstanceID(SDL_JoystickID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickFromInstanceID");
    if (!IsWindows()) {
        SDL_Joystick *(*local)(SDL_JoystickID a) = addr;
        return local(a);
    } else {
        SDL_Joystick *__attribute__((__ms_abi__)) (*local)(SDL_JoystickID a) = addr;
        return local(a);
    }
}
int abi_SDL_GetDisplayUsableBounds(int a, SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayUsableBounds");
    if (!IsWindows()) {
        int (*local)(int a, SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, SDL_Rect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetWindowBordersSize(SDL_Window *a, int *b, int *c, int *d, int *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowBordersSize");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, int *b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c, int *d, int *e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_SetWindowOpacity(SDL_Window *a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowOpacity");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, float b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, float b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetWindowOpacity(SDL_Window *a, float *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowOpacity");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, float *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, float *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetWindowInputFocus(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowInputFocus");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_SetWindowModalFor(SDL_Window *a, SDL_Window *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowModalFor");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_Window * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_Window * b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderSetIntegerScale(SDL_Renderer *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetIntegerScale");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_bool b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_bool b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_RenderGetIntegerScale(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetIntegerScale");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
Uint32 abi_SDL_DequeueAudio(SDL_AudioDeviceID a, void *b, Uint32 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_DequeueAudio");
    if (!IsWindows()) {
        Uint32 (*local)(SDL_AudioDeviceID a, void *b, Uint32 c) = addr;
        return local(a, b, c);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(SDL_AudioDeviceID a, void *b, Uint32 c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_SetWindowResizable(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowResizable");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
SDL_Surface *abi_SDL_CreateRGBSurfaceWithFormat(Uint32 a, int b, int c, int d, Uint32 e)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateRGBSurfaceWithFormat");
    if (!IsWindows()) {
        SDL_Surface *(*local)(Uint32 a, int b, int c, int d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(Uint32 a, int b, int c, int d, Uint32 e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_Surface *abi_SDL_CreateRGBSurfaceWithFormatFrom(void *a, int b, int c, int d, int e, Uint32 f)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateRGBSurfaceWithFormatFrom");
    if (!IsWindows()) {
        SDL_Surface *(*local)(void *a, int b, int c, int d, int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(void *a, int b, int c, int d, int e, Uint32 f) = addr;
        return local(a, b, c, d, e, f);
    }
}
SDL_bool abi_SDL_GetHintBoolean(const char *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetHintBoolean");
    if (!IsWindows()) {
        SDL_bool (*local)(const char *a, SDL_bool b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const char *a, SDL_bool b) = addr;
        return local(a, b);
    }
}
Uint16 abi_SDL_JoystickGetDeviceVendor(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceVendor");
    if (!IsWindows()) {
        Uint16 (*local)(int a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetDeviceProduct(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceProduct");
    if (!IsWindows()) {
        Uint16 (*local)(int a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetDeviceProductVersion(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceProductVersion");
    if (!IsWindows()) {
        Uint16 (*local)(int a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetVendor(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetVendor");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetProduct(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetProduct");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetProductVersion(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetProductVersion");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_GameControllerGetVendor(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetVendor");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_GameControllerGetProduct(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetProduct");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_GameControllerGetProductVersion(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetProductVersion");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasNEON(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasNEON");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GameControllerNumMappings(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerNumMappings");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
char *abi_SDL_GameControllerMappingForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerMappingForIndex");
    if (!IsWindows()) {
        char *(*local)(int a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_JoystickGetAxisInitialState(SDL_Joystick *a, int b, Sint16 *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetAxisInitialState");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Joystick * a, int b, Sint16 *c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b, Sint16 *c) = addr;
        return local(a, b, c);
    }
}
SDL_JoystickType abi_SDL_JoystickGetDeviceType(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceType");
    if (!IsWindows()) {
        SDL_JoystickType (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_JoystickType __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_JoystickType abi_SDL_JoystickGetType(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetType");
    if (!IsWindows()) {
        SDL_JoystickType (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_JoystickType __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
void abi_SDL_MemoryBarrierReleaseFunction(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_MemoryBarrierReleaseFunction");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_MemoryBarrierAcquireFunction(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_MemoryBarrierAcquireFunction");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_JoystickID abi_SDL_JoystickGetDeviceInstanceID(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDeviceInstanceID");
    if (!IsWindows()) {
        SDL_JoystickID (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_JoystickID __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
size_t abi_SDL_utf8strlen(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_utf8strlen");
    if (!IsWindows()) {
        size_t (*local)(const char *a) = addr;
        return local(a);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
void *abi_SDL_LoadFile_RW(SDL_RWops *a, size_t *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadFile_RW");
    if (!IsWindows()) {
        void *(*local)(SDL_RWops * a, size_t * b, int c) = addr;
        return local(a, b, c);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_RWops * a, size_t * b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_wcscmp(const wchar_t *a, const wchar_t *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcscmp");
    if (!IsWindows()) {
        int (*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    }
}
SDL_BlendMode abi_SDL_ComposeCustomBlendMode(SDL_BlendFactor a, SDL_BlendFactor b, SDL_BlendOperation c, SDL_BlendFactor d, SDL_BlendFactor e, SDL_BlendOperation f)
{
    void *addr = cosmo_dlsym(libD, "SDL_ComposeCustomBlendMode");
    if (!IsWindows()) {
        SDL_BlendMode (*local)(SDL_BlendFactor a, SDL_BlendFactor b, SDL_BlendOperation c, SDL_BlendFactor d, SDL_BlendFactor e, SDL_BlendOperation f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_BlendMode __attribute__((__ms_abi__)) (*local)(SDL_BlendFactor a, SDL_BlendFactor b, SDL_BlendOperation c, SDL_BlendFactor d, SDL_BlendFactor e, SDL_BlendOperation f) = addr;
        return local(a, b, c, d, e, f);
    }
}
SDL_Surface *abi_SDL_DuplicateSurface(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DuplicateSurface");
    if (!IsWindows()) {
        SDL_Surface *(*local)(SDL_Surface * a) = addr;
        return local(a);
    } else {
        SDL_Surface *__attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        return local(a);
    }
}
void abi_SDL_LockJoysticks(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockJoysticks");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_UnlockJoysticks(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockJoysticks");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_GetMemoryFunctions(SDL_malloc_func *a, SDL_calloc_func *b, SDL_realloc_func *c, SDL_free_func *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetMemoryFunctions");
    if (!IsWindows()) {
        void (*local)(SDL_malloc_func * a, SDL_calloc_func * b, SDL_realloc_func * c, SDL_free_func * d) = addr;
        local(a, b, c, d);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_malloc_func * a, SDL_calloc_func * b, SDL_realloc_func * c, SDL_free_func * d) = addr;
        local(a, b, c, d);
    }
}
int abi_SDL_SetMemoryFunctions(SDL_malloc_func a, SDL_calloc_func b, SDL_realloc_func c, SDL_free_func d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetMemoryFunctions");
    if (!IsWindows()) {
        int (*local)(SDL_malloc_func a, SDL_calloc_func b, SDL_realloc_func c, SDL_free_func d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_malloc_func a, SDL_calloc_func b, SDL_realloc_func c, SDL_free_func d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_GetNumAllocations(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetNumAllocations");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_AudioStream *abi_SDL_NewAudioStream(const SDL_AudioFormat a, const Uint8 b, const int c, const SDL_AudioFormat d, const Uint8 e, const int f)
{
    void *addr = cosmo_dlsym(libD, "SDL_NewAudioStream");
    if (!IsWindows()) {
        SDL_AudioStream *(*local)(const SDL_AudioFormat a, const Uint8 b, const int c, const SDL_AudioFormat d, const Uint8 e, const int f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        SDL_AudioStream *__attribute__((__ms_abi__)) (*local)(const SDL_AudioFormat a, const Uint8 b, const int c, const SDL_AudioFormat d, const Uint8 e, const int f) = addr;
        return local(a, b, c, d, e, f);
    }
}
int abi_SDL_AudioStreamPut(SDL_AudioStream *a, const void *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioStreamPut");
    if (!IsWindows()) {
        int (*local)(SDL_AudioStream * a, const void *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a, const void *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_AudioStreamGet(SDL_AudioStream *a, void *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioStreamGet");
    if (!IsWindows()) {
        int (*local)(SDL_AudioStream * a, void *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a, void *b, int c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_AudioStreamClear(SDL_AudioStream *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioStreamClear");
    if (!IsWindows()) {
        void (*local)(SDL_AudioStream * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a) = addr;
        local(a);
    }
}
int abi_SDL_AudioStreamAvailable(SDL_AudioStream *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioStreamAvailable");
    if (!IsWindows()) {
        int (*local)(SDL_AudioStream * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a) = addr;
        return local(a);
    }
}
void abi_SDL_FreeAudioStream(SDL_AudioStream *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_FreeAudioStream");
    if (!IsWindows()) {
        void (*local)(SDL_AudioStream * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a) = addr;
        local(a);
    }
}
int abi_SDL_AudioStreamFlush(SDL_AudioStream *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_AudioStreamFlush");
    if (!IsWindows()) {
        int (*local)(SDL_AudioStream * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_AudioStream * a) = addr;
        return local(a);
    }
}
float abi_SDL_acosf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_acosf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_asinf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_asinf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_atanf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_atanf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_atan2f(float a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_atan2f");
    if (!IsWindows()) {
        float (*local)(float a, float b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a, float b) = addr;
        return local(a, b);
    }
}
float abi_SDL_ceilf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ceilf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_copysignf(float a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_copysignf");
    if (!IsWindows()) {
        float (*local)(float a, float b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a, float b) = addr;
        return local(a, b);
    }
}
float abi_SDL_fabsf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_fabsf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_floorf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_floorf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_logf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_logf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
float abi_SDL_powf(float a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_powf");
    if (!IsWindows()) {
        float (*local)(float a, float b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a, float b) = addr;
        return local(a, b);
    }
}
float abi_SDL_scalbnf(float a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_scalbnf");
    if (!IsWindows()) {
        float (*local)(float a, int b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a, int b) = addr;
        return local(a, b);
    }
}
double abi_SDL_fmod(double a, double b)
{
    void *addr = cosmo_dlsym(libD, "SDL_fmod");
    if (!IsWindows()) {
        double (*local)(double a, double b) = addr;
        return local(a, b);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a, double b) = addr;
        return local(a, b);
    }
}
float abi_SDL_fmodf(float a, float b)
{
    void *addr = cosmo_dlsym(libD, "SDL_fmodf");
    if (!IsWindows()) {
        float (*local)(float a, float b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a, float b) = addr;
        return local(a, b);
    }
}
void abi_SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_MODE a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetYUVConversionMode");
    if (!IsWindows()) {
        void (*local)(SDL_YUV_CONVERSION_MODE a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_YUV_CONVERSION_MODE a) = addr;
        local(a);
    }
}
SDL_YUV_CONVERSION_MODE abi_SDL_GetYUVConversionMode(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetYUVConversionMode");
    if (!IsWindows()) {
        SDL_YUV_CONVERSION_MODE (*local)
        (void) = addr;
        return local();
    } else {
        SDL_YUV_CONVERSION_MODE __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_YUV_CONVERSION_MODE abi_SDL_GetYUVConversionModeForResolution(int a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetYUVConversionModeForResolution");
    if (!IsWindows()) {
        SDL_YUV_CONVERSION_MODE (*local)
        (int a, int b) = addr;
        return local(a, b);
    } else {
        SDL_YUV_CONVERSION_MODE __attribute__((__ms_abi__)) (*local)(int a, int b) = addr;
        return local(a, b);
    }
}
void *abi_SDL_RenderGetMetalLayer(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetMetalLayer");
    if (!IsWindows()) {
        void *(*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
void *abi_SDL_RenderGetMetalCommandEncoder(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetMetalCommandEncoder");
    if (!IsWindows()) {
        void *(*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
double abi_SDL_log10(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_log10");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_log10f(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_log10f");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
char *abi_SDL_GameControllerMappingForDeviceIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerMappingForDeviceIndex");
    if (!IsWindows()) {
        char *(*local)(int a) = addr;
        return local(a);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasAVX512F(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasAVX512F");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
double abi_SDL_exp(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_exp");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_expf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_expf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
wchar_t *abi_SDL_wcsdup(const wchar_t *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcsdup");
    if (!IsWindows()) {
        wchar_t *(*local)(const wchar_t *a) = addr;
        return local(a);
    } else {
        wchar_t *__attribute__((__ms_abi__)) (*local)(const wchar_t *a) = addr;
        return local(a);
    }
}
int abi_SDL_GameControllerRumble(SDL_GameController *a, Uint16 b, Uint16 c, Uint32 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerRumble");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_JoystickRumble(SDL_Joystick *a, Uint16 b, Uint16 c, Uint32 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickRumble");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_NumSensors(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_NumSensors");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_SensorGetDeviceName(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetDeviceName");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_SensorType abi_SDL_SensorGetDeviceType(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetDeviceType");
    if (!IsWindows()) {
        SDL_SensorType (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_SensorType __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_SensorGetDeviceNonPortableType(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetDeviceNonPortableType");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_SensorID abi_SDL_SensorGetDeviceInstanceID(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetDeviceInstanceID");
    if (!IsWindows()) {
        SDL_SensorID (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_SensorID __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_Sensor *abi_SDL_SensorOpen(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorOpen");
    if (!IsWindows()) {
        SDL_Sensor *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_Sensor *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_Sensor *abi_SDL_SensorFromInstanceID(SDL_SensorID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorFromInstanceID");
    if (!IsWindows()) {
        SDL_Sensor *(*local)(SDL_SensorID a) = addr;
        return local(a);
    } else {
        SDL_Sensor *__attribute__((__ms_abi__)) (*local)(SDL_SensorID a) = addr;
        return local(a);
    }
}
const char *abi_SDL_SensorGetName(SDL_Sensor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetName");
    if (!IsWindows()) {
        const char *(*local)(SDL_Sensor * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Sensor * a) = addr;
        return local(a);
    }
}
SDL_SensorType abi_SDL_SensorGetType(SDL_Sensor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetType");
    if (!IsWindows()) {
        SDL_SensorType (*local)(SDL_Sensor * a) = addr;
        return local(a);
    } else {
        SDL_SensorType __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a) = addr;
        return local(a);
    }
}
int abi_SDL_SensorGetNonPortableType(SDL_Sensor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetNonPortableType");
    if (!IsWindows()) {
        int (*local)(SDL_Sensor * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a) = addr;
        return local(a);
    }
}
SDL_SensorID abi_SDL_SensorGetInstanceID(SDL_Sensor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetInstanceID");
    if (!IsWindows()) {
        SDL_SensorID (*local)(SDL_Sensor * a) = addr;
        return local(a);
    } else {
        SDL_SensorID __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a) = addr;
        return local(a);
    }
}
int abi_SDL_SensorGetData(SDL_Sensor *a, float *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetData");
    if (!IsWindows()) {
        int (*local)(SDL_Sensor * a, float *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a, float *b, int c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_SensorClose(SDL_Sensor *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorClose");
    if (!IsWindows()) {
        void (*local)(SDL_Sensor * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a) = addr;
        local(a);
    }
}
void abi_SDL_SensorUpdate(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorUpdate");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_IsTablet(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsTablet");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_DisplayOrientation abi_SDL_GetDisplayOrientation(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDisplayOrientation");
    if (!IsWindows()) {
        SDL_DisplayOrientation (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_DisplayOrientation __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasColorKey(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasColorKey");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Surface * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        return local(a);
    }
}
SDL_Thread *abi_SDL_CreateThreadWithStackSize(SDL_ThreadFunction a, const char *b, const size_t c, void *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_CreateThreadWithStackSize");
    if (!IsWindows()) {
        SDL_Thread *(*local)(SDL_ThreadFunction a, const char *b, const size_t c, void *d) = addr;
        return local(a, b, c, d);
    } else {
        SDL_Thread *__attribute__((__ms_abi__)) (*local)(SDL_ThreadFunction a, const char *b, const size_t c, void *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_JoystickGetDevicePlayerIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetDevicePlayerIndex");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickGetPlayerIndex(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetPlayerIndex");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_GameControllerGetPlayerIndex(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetPlayerIndex");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
int abi_SDL_RenderFlush(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderFlush");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
int abi_SDL_RenderDrawPointF(SDL_Renderer *a, float b, float c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawPointF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, float b, float c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, float b, float c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawPointsF(SDL_Renderer *a, const SDL_FPoint *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawPointsF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FPoint *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FPoint *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawLineF(SDL_Renderer *a, float b, float c, float d, float e)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawLineF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, float b, float c, float d, float e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, float b, float c, float d, float e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_RenderDrawLinesF(SDL_Renderer *a, const SDL_FPoint *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawLinesF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FPoint *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FPoint *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderDrawRectF(SDL_Renderer *a, const SDL_FRect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawRectF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FRect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FRect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderDrawRectsF(SDL_Renderer *a, const SDL_FRect *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderDrawRectsF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FRect *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FRect *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderFillRectF(SDL_Renderer *a, const SDL_FRect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderFillRectF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FRect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FRect *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_RenderFillRectsF(SDL_Renderer *a, const SDL_FRect *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderFillRectsF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, const SDL_FRect *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, const SDL_FRect *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_RenderCopyF(SDL_Renderer *a, SDL_Texture *b, const SDL_Rect *c, const SDL_FRect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderCopyF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_FRect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_FRect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_RenderCopyExF(SDL_Renderer *a, SDL_Texture *b, const SDL_Rect *c, const SDL_FRect *d, const double e, const SDL_FPoint *f, const SDL_RendererFlip g)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderCopyExF");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_FRect *d, const double e, const SDL_FPoint *f, const SDL_RendererFlip g) = addr;
        return local(a, b, c, d, e, f, g);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Rect *c, const SDL_FRect *d, const double e, const SDL_FPoint *f, const SDL_RendererFlip g) = addr;
        return local(a, b, c, d, e, f, g);
    }
}
SDL_TouchDeviceType abi_SDL_GetTouchDeviceType(SDL_TouchID a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTouchDeviceType");
    if (!IsWindows()) {
        SDL_TouchDeviceType (*local)(SDL_TouchID a) = addr;
        return local(a);
    } else {
        SDL_TouchDeviceType __attribute__((__ms_abi__)) (*local)(SDL_TouchID a) = addr;
        return local(a);
    }
}
size_t abi_SDL_SIMDGetAlignment(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_SIMDGetAlignment");
    if (!IsWindows()) {
        size_t (*local)(void) = addr;
        return local();
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void *abi_SDL_SIMDAlloc(const size_t a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SIMDAlloc");
    if (!IsWindows()) {
        void *(*local)(const size_t a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(const size_t a) = addr;
        return local(a);
    }
}
void abi_SDL_SIMDFree(void *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SIMDFree");
    if (!IsWindows()) {
        void (*local)(void *a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(void *a) = addr;
        local(a);
    }
}
Sint64 abi_SDL_RWsize(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWsize");
    if (!IsWindows()) {
        Sint64 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Sint64 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
Sint64 abi_SDL_RWseek(SDL_RWops *a, Sint64 b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWseek");
    if (!IsWindows()) {
        Sint64 (*local)(SDL_RWops * a, Sint64 b, int c) = addr;
        return local(a, b, c);
    } else {
        Sint64 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, Sint64 b, int c) = addr;
        return local(a, b, c);
    }
}
Sint64 abi_SDL_RWtell(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWtell");
    if (!IsWindows()) {
        Sint64 (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        Sint64 __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
size_t abi_SDL_RWread(SDL_RWops *a, void *b, size_t c, size_t d)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWread");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, void *b, size_t c, size_t d) = addr;
        return local(a, b, c, d);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, void *b, size_t c, size_t d) = addr;
        return local(a, b, c, d);
    }
}
size_t abi_SDL_RWwrite(SDL_RWops *a, const void *b, size_t c, size_t d)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWwrite");
    if (!IsWindows()) {
        size_t (*local)(SDL_RWops * a, const void *b, size_t c, size_t d) = addr;
        return local(a, b, c, d);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(SDL_RWops * a, const void *b, size_t c, size_t d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_RWclose(SDL_RWops *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RWclose");
    if (!IsWindows()) {
        int (*local)(SDL_RWops * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_RWops * a) = addr;
        return local(a);
    }
}
void *abi_SDL_LoadFile(const char *a, size_t *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_LoadFile");
    if (!IsWindows()) {
        void *(*local)(const char *a, size_t *b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(const char *a, size_t *b) = addr;
        return local(a, b);
    }
}
SDL_MetalView abi_SDL_Metal_CreateView(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Metal_CreateView");
    if (!IsWindows()) {
        SDL_MetalView (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_MetalView __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
void abi_SDL_Metal_DestroyView(SDL_MetalView a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Metal_DestroyView");
    if (!IsWindows()) {
        void (*local)(SDL_MetalView a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_MetalView a) = addr;
        local(a);
    }
}
int abi_SDL_LockTextureToSurface(SDL_Texture *a, const SDL_Rect *b, SDL_Surface **c)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockTextureToSurface");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, const SDL_Rect *b, SDL_Surface **c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, const SDL_Rect *b, SDL_Surface **c) = addr;
        return local(a, b, c);
    }
}
SDL_bool abi_SDL_HasARMSIMD(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasARMSIMD");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
char *abi_SDL_strtokr(char *a, const char *b, char **c)
{
    void *addr = cosmo_dlsym(libD, "SDL_strtokr");
    if (!IsWindows()) {
        char *(*local)(char *a, const char *b, char **c) = addr;
        return local(a, b, c);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(char *a, const char *b, char **c) = addr;
        return local(a, b, c);
    }
}
wchar_t *abi_SDL_wcsstr(const wchar_t *a, const wchar_t *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcsstr");
    if (!IsWindows()) {
        wchar_t *(*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    } else {
        wchar_t *__attribute__((__ms_abi__)) (*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_wcsncmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcsncmp");
    if (!IsWindows()) {
        int (*local)(const wchar_t *a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const wchar_t *a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    }
}
SDL_GameControllerType abi_SDL_GameControllerTypeForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerTypeForIndex");
    if (!IsWindows()) {
        SDL_GameControllerType (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_GameControllerType __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_GameControllerType abi_SDL_GameControllerGetType(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetType");
    if (!IsWindows()) {
        SDL_GameControllerType (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_GameControllerType __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_GameController *abi_SDL_GameControllerFromPlayerIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerFromPlayerIndex");
    if (!IsWindows()) {
        SDL_GameController *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_GameController *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void abi_SDL_GameControllerSetPlayerIndex(SDL_GameController *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerSetPlayerIndex");
    if (!IsWindows()) {
        void (*local)(SDL_GameController * a, int b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, int b) = addr;
        local(a, b);
    }
}
SDL_Joystick *abi_SDL_JoystickFromPlayerIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickFromPlayerIndex");
    if (!IsWindows()) {
        SDL_Joystick *(*local)(int a) = addr;
        return local(a);
    } else {
        SDL_Joystick *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void abi_SDL_JoystickSetPlayerIndex(SDL_Joystick *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSetPlayerIndex");
    if (!IsWindows()) {
        void (*local)(SDL_Joystick * a, int b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b) = addr;
        local(a, b);
    }
}
int abi_SDL_SetTextureScaleMode(SDL_Texture *a, SDL_ScaleMode b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextureScaleMode");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, SDL_ScaleMode b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, SDL_ScaleMode b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GetTextureScaleMode(SDL_Texture *a, SDL_ScaleMode *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTextureScaleMode");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, SDL_ScaleMode * b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, SDL_ScaleMode * b) = addr;
        return local(a, b);
    }
}
void abi_SDL_OnApplicationWillTerminate(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationWillTerminate");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_OnApplicationDidReceiveMemoryWarning(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationDidReceiveMemoryWarning");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_OnApplicationWillResignActive(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationWillResignActive");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_OnApplicationDidEnterBackground(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationDidEnterBackground");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_OnApplicationWillEnterForeground(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationWillEnterForeground");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_OnApplicationDidBecomeActive(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_OnApplicationDidBecomeActive");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_isupper(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isupper");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_islower(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_islower");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickAttachVirtual(SDL_JoystickType a, int b, int c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickAttachVirtual");
    if (!IsWindows()) {
        int (*local)(SDL_JoystickType a, int b, int c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_JoystickType a, int b, int c, int d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_JoystickDetachVirtual(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickDetachVirtual");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_JoystickIsVirtual(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickIsVirtual");
    if (!IsWindows()) {
        SDL_bool (*local)(int a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickSetVirtualAxis(SDL_Joystick *a, int b, Sint16 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSetVirtualAxis");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, int b, Sint16 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b, Sint16 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_JoystickSetVirtualButton(SDL_Joystick *a, int b, Uint8 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSetVirtualButton");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, int b, Uint8 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b, Uint8 c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_JoystickSetVirtualHat(SDL_Joystick *a, int b, Uint8 c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSetVirtualHat");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, int b, Uint8 c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, int b, Uint8 c) = addr;
        return local(a, b, c);
    }
}
char *abi_SDL_GetErrorMsg(char *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetErrorMsg");
    if (!IsWindows()) {
        char *(*local)(char *a, int b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(char *a, int b) = addr;
        return local(a, b);
    }
}
void abi_SDL_LockSensors(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_LockSensors");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_UnlockSensors(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnlockSensors");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void *abi_SDL_Metal_GetLayer(SDL_MetalView a)
{
    void *addr = cosmo_dlsym(libD, "SDL_Metal_GetLayer");
    if (!IsWindows()) {
        void *(*local)(SDL_MetalView a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_MetalView a) = addr;
        return local(a);
    }
}
void abi_SDL_Metal_GetDrawableSize(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_Metal_GetDrawableSize");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
double abi_SDL_trunc(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_trunc");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_truncf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_truncf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
SDL_Locale *abi_SDL_GetPreferredLocales(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPreferredLocales");
    if (!IsWindows()) {
        SDL_Locale *(*local)(void) = addr;
        return local();
    } else {
        SDL_Locale *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
void *abi_SDL_SIMDRealloc(void *a, const size_t b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SIMDRealloc");
    if (!IsWindows()) {
        void *(*local)(void *a, const size_t b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(void *a, const size_t b) = addr;
        return local(a, b);
    }
}
int abi_SDL_OpenURL(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_OpenURL");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasSurfaceRLE(SDL_Surface *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasSurfaceRLE");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Surface * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Surface * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GameControllerHasLED(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasLED");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
int abi_SDL_GameControllerSetLED(SDL_GameController *a, Uint8 b, Uint8 c, Uint8 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerSetLED");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    }
}
SDL_bool abi_SDL_JoystickHasLED(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickHasLED");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickSetLED(SDL_Joystick *a, Uint8 b, Uint8 c, Uint8 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSetLED");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, Uint8 b, Uint8 c, Uint8 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_GameControllerRumbleTriggers(SDL_GameController *a, Uint16 b, Uint16 c, Uint32 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerRumbleTriggers");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_JoystickRumbleTriggers(SDL_Joystick *a, Uint16 b, Uint16 c, Uint32 d)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickRumbleTriggers");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, Uint16 b, Uint16 c, Uint32 d) = addr;
        return local(a, b, c, d);
    }
}
SDL_bool abi_SDL_GameControllerHasAxis(SDL_GameController *a, SDL_GameControllerAxis b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasAxis");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_GameControllerHasButton(SDL_GameController *a, SDL_GameControllerButton b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasButton");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerGetNumTouchpads(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetNumTouchpads");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
int abi_SDL_GameControllerGetNumTouchpadFingers(SDL_GameController *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetNumTouchpadFingers");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerGetTouchpadFinger(SDL_GameController *a, int b, int c, Uint8 *d, float *e, float *f, float *g)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetTouchpadFinger");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, int b, int c, Uint8 *d, float *e, float *f, float *g) = addr;
        return local(a, b, c, d, e, f, g);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, int b, int c, Uint8 *d, float *e, float *f, float *g) = addr;
        return local(a, b, c, d, e, f, g);
    }
}
Uint32 abi_SDL_crc32(Uint32 a, const void *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_crc32");
    if (!IsWindows()) {
        Uint32 (*local)(Uint32 a, const void *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(Uint32 a, const void *b, size_t c) = addr;
        return local(a, b, c);
    }
}
const char *abi_SDL_GameControllerGetSerial(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetSerial");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
const char *abi_SDL_JoystickGetSerial(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetSerial");
    if (!IsWindows()) {
        const char *(*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GameControllerHasSensor(SDL_GameController *a, SDL_SensorType b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasSensor");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerSetSensorEnabled(SDL_GameController *a, SDL_SensorType b, SDL_bool c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerSetSensorEnabled");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, SDL_SensorType b, SDL_bool c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b, SDL_bool c) = addr;
        return local(a, b, c);
    }
}
SDL_bool abi_SDL_GameControllerIsSensorEnabled(SDL_GameController *a, SDL_SensorType b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerIsSensorEnabled");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerGetSensorData(SDL_GameController *a, SDL_SensorType b, float *c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetSensorData");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, SDL_SensorType b, float *c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b, float *c, int d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_wcscasecmp(const wchar_t *a, const wchar_t *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcscasecmp");
    if (!IsWindows()) {
        int (*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const wchar_t *a, const wchar_t *b) = addr;
        return local(a, b);
    }
}
int abi_SDL_wcsncasecmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_wcsncasecmp");
    if (!IsWindows()) {
        int (*local)(const wchar_t *a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const wchar_t *a, const wchar_t *b, size_t c) = addr;
        return local(a, b, c);
    }
}
double abi_SDL_round(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_round");
    if (!IsWindows()) {
        double (*local)(double a) = addr;
        return local(a);
    } else {
        double __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
float abi_SDL_roundf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_roundf");
    if (!IsWindows()) {
        float (*local)(float a) = addr;
        return local(a);
    } else {
        float __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
long abi_SDL_lround(double a)
{
    void *addr = cosmo_dlsym(libD, "SDL_lround");
    if (!IsWindows()) {
        long (*local)(double a) = addr;
        return local(a);
    } else {
        long __attribute__((__ms_abi__)) (*local)(double a) = addr;
        return local(a);
    }
}
long abi_SDL_lroundf(float a)
{
    void *addr = cosmo_dlsym(libD, "SDL_lroundf");
    if (!IsWindows()) {
        long (*local)(float a) = addr;
        return local(a);
    } else {
        long __attribute__((__ms_abi__)) (*local)(float a) = addr;
        return local(a);
    }
}
int abi_SDL_SoftStretchLinear(SDL_Surface *a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SoftStretchLinear");
    if (!IsWindows()) {
        int (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Surface * a, const SDL_Rect *b, SDL_Surface *c, const SDL_Rect *d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_UpdateNVTexture(SDL_Texture *a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f)
{
    void *addr = cosmo_dlsym(libD, "SDL_UpdateNVTexture");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, const SDL_Rect *b, const Uint8 *c, int d, const Uint8 *e, int f) = addr;
        return local(a, b, c, d, e, f);
    }
}
void abi_SDL_SetWindowKeyboardGrab(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowKeyboardGrab");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
void abi_SDL_SetWindowMouseGrab(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowMouseGrab");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
SDL_bool abi_SDL_GetWindowKeyboardGrab(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowKeyboardGrab");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GetWindowMouseGrab(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowMouseGrab");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_isalpha(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isalpha");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isalnum(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isalnum");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isblank(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isblank");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_iscntrl(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_iscntrl");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isxdigit(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isxdigit");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_ispunct(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ispunct");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isprint(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isprint");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_isgraph(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_isgraph");
    if (!IsWindows()) {
        int (*local)(int a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
int abi_SDL_GetAudioDeviceSpec(int a, int b, SDL_AudioSpec *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetAudioDeviceSpec");
    if (!IsWindows()) {
        int (*local)(int a, int b, SDL_AudioSpec *c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b, SDL_AudioSpec *c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_TLSCleanup(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_TLSCleanup");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
void abi_SDL_SetWindowAlwaysOnTop(SDL_Window *a, SDL_bool b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowAlwaysOnTop");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_bool b) = addr;
        local(a, b);
    }
}
int abi_SDL_FlashWindow(SDL_Window *a, SDL_FlashOperation b)
{
    void *addr = cosmo_dlsym(libD, "SDL_FlashWindow");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, SDL_FlashOperation b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, SDL_FlashOperation b) = addr;
        return local(a, b);
    }
}
int abi_SDL_GameControllerSendEffect(SDL_GameController *a, const void *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerSendEffect");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, const void *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, const void *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_JoystickSendEffect(SDL_Joystick *a, const void *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickSendEffect");
    if (!IsWindows()) {
        int (*local)(SDL_Joystick * a, const void *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a, const void *b, int c) = addr;
        return local(a, b, c);
    }
}
float abi_SDL_GameControllerGetSensorDataRate(SDL_GameController *a, SDL_SensorType b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetSensorDataRate");
    if (!IsWindows()) {
        float (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    } else {
        float __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b) = addr;
        return local(a, b);
    }
}
int abi_SDL_SetTextureUserData(SDL_Texture *a, void *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetTextureUserData");
    if (!IsWindows()) {
        int (*local)(SDL_Texture * a, void *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Texture * a, void *b) = addr;
        return local(a, b);
    }
}
void *abi_SDL_GetTextureUserData(SDL_Texture *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTextureUserData");
    if (!IsWindows()) {
        void *(*local)(SDL_Texture * a) = addr;
        return local(a);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Texture * a) = addr;
        return local(a);
    }
}
int abi_SDL_RenderGeometry(SDL_Renderer *a, SDL_Texture *b, const SDL_Vertex *c, int d, const int *e, int f)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGeometry");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Vertex *c, int d, const int *e, int f) = addr;
        return local(a, b, c, d, e, f);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const SDL_Vertex *c, int d, const int *e, int f) = addr;
        return local(a, b, c, d, e, f);
    }
}
int abi_SDL_RenderGeometryRaw(SDL_Renderer *a, SDL_Texture *b, const float *c, int d, const SDL_Color *e, int f, const float *g, int h, int i, const void *j, int k, int l)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGeometryRaw");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, SDL_Texture * b, const float *c, int d, const SDL_Color *e, int f, const float *g, int h, int i, const void *j, int k, int l) = addr;
        return local(a, b, c, d, e, f, g, h, i, j, k, l);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, SDL_Texture * b, const float *c, int d, const SDL_Color *e, int f, const float *g, int h, int i, const void *j, int k, int l) = addr;
        return local(a, b, c, d, e, f, g, h, i, j, k, l);
    }
}
int abi_SDL_RenderSetVSync(SDL_Renderer *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderSetVSync");
    if (!IsWindows()) {
        int (*local)(SDL_Renderer * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_vasprintf(char **a, const char *b, va_list c)
{
    void *addr = cosmo_dlsym(libD, "SDL_vasprintf");
    if (!IsWindows()) {
        int (*local)(char **a, const char *b, va_list c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(char **a, const char *b, va_list c) = addr;
        return local(a, b, c);
    }
}
void *abi_SDL_GetWindowICCProfile(SDL_Window *a, size_t *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowICCProfile");
    if (!IsWindows()) {
        void *(*local)(SDL_Window * a, size_t * b) = addr;
        return local(a, b);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(SDL_Window * a, size_t * b) = addr;
        return local(a, b);
    }
}
Uint64 abi_SDL_GetTicks64(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTicks64");
    if (!IsWindows()) {
        Uint64 (*local)(void) = addr;
        return local();
    } else {
        Uint64 __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
const char *abi_SDL_GameControllerGetAppleSFSymbolsNameForButton(SDL_GameController *a, SDL_GameControllerButton b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetAppleSFSymbolsNameForButton");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerButton b) = addr;
        return local(a, b);
    }
}
const char *abi_SDL_GameControllerGetAppleSFSymbolsNameForAxis(SDL_GameController *a, SDL_GameControllerAxis b)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetAppleSFSymbolsNameForAxis");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_GameControllerAxis b) = addr;
        return local(a, b);
    }
}
int abi_SDL_hid_init(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_init");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_hid_exit(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_exit");
    if (!IsWindows()) {
        int (*local)(void) = addr;
        return local();
    } else {
        int __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
Uint32 abi_SDL_hid_device_change_count(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_device_change_count");
    if (!IsWindows()) {
        Uint32 (*local)(void) = addr;
        return local();
    } else {
        Uint32 __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_hid_device_info *abi_SDL_hid_enumerate(unsigned short a, unsigned short b)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_enumerate");
    if (!IsWindows()) {
        SDL_hid_device_info *(*local)(unsigned short a, unsigned short b) = addr;
        return local(a, b);
    } else {
        SDL_hid_device_info *__attribute__((__ms_abi__)) (*local)(unsigned short a, unsigned short b) = addr;
        return local(a, b);
    }
}
void abi_SDL_hid_free_enumeration(SDL_hid_device_info *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_free_enumeration");
    if (!IsWindows()) {
        void (*local)(SDL_hid_device_info * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_hid_device_info * a) = addr;
        local(a);
    }
}
SDL_hid_device *abi_SDL_hid_open(unsigned short a, unsigned short b, const wchar_t *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_open");
    if (!IsWindows()) {
        SDL_hid_device *(*local)(unsigned short a, unsigned short b, const wchar_t *c) = addr;
        return local(a, b, c);
    } else {
        SDL_hid_device *__attribute__((__ms_abi__)) (*local)(unsigned short a, unsigned short b, const wchar_t *c) = addr;
        return local(a, b, c);
    }
}
SDL_hid_device *abi_SDL_hid_open_path(const char *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_open_path");
    if (!IsWindows()) {
        SDL_hid_device *(*local)(const char *a, int b) = addr;
        return local(a, b);
    } else {
        SDL_hid_device *__attribute__((__ms_abi__)) (*local)(const char *a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_hid_write(SDL_hid_device *a, const unsigned char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_write");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, const unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, const unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_read_timeout(SDL_hid_device *a, unsigned char *b, size_t c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_read_timeout");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, unsigned char *b, size_t c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, unsigned char *b, size_t c, int d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_hid_read(SDL_hid_device *a, unsigned char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_read");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_set_nonblocking(SDL_hid_device *a, int b)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_set_nonblocking");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, int b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, int b) = addr;
        return local(a, b);
    }
}
int abi_SDL_hid_send_feature_report(SDL_hid_device *a, const unsigned char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_send_feature_report");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, const unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, const unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_get_feature_report(SDL_hid_device *a, unsigned char *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_get_feature_report");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, unsigned char *b, size_t c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_hid_close(SDL_hid_device *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_close");
    if (!IsWindows()) {
        void (*local)(SDL_hid_device * a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a) = addr;
        local(a);
    }
}
int abi_SDL_hid_get_manufacturer_string(SDL_hid_device *a, wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_get_manufacturer_string");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_get_product_string(SDL_hid_device *a, wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_get_product_string");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_get_serial_number_string(SDL_hid_device *a, wchar_t *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_get_serial_number_string");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, wchar_t * b, size_t c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_hid_get_indexed_string(SDL_hid_device *a, int b, wchar_t *c, size_t d)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_get_indexed_string");
    if (!IsWindows()) {
        int (*local)(SDL_hid_device * a, int b, wchar_t *c, size_t d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_hid_device * a, int b, wchar_t *c, size_t d) = addr;
        return local(a, b, c, d);
    }
}
int abi_SDL_SetWindowMouseRect(SDL_Window *a, const SDL_Rect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetWindowMouseRect");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a, const SDL_Rect *b) = addr;
        return local(a, b);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a, const SDL_Rect *b) = addr;
        return local(a, b);
    }
}
const SDL_Rect *abi_SDL_GetWindowMouseRect(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowMouseRect");
    if (!IsWindows()) {
        const SDL_Rect *(*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        const SDL_Rect *__attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
void abi_SDL_RenderWindowToLogical(SDL_Renderer *a, int b, int c, float *d, float *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderWindowToLogical");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, int b, int c, float *d, float *e) = addr;
        local(a, b, c, d, e);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, int b, int c, float *d, float *e) = addr;
        local(a, b, c, d, e);
    }
}
void abi_SDL_RenderLogicalToWindow(SDL_Renderer *a, float b, float c, int *d, int *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderLogicalToWindow");
    if (!IsWindows()) {
        void (*local)(SDL_Renderer * a, float b, float c, int *d, int *e) = addr;
        local(a, b, c, d, e);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Renderer * a, float b, float c, int *d, int *e) = addr;
        local(a, b, c, d, e);
    }
}
SDL_bool abi_SDL_JoystickHasRumble(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickHasRumble");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_JoystickHasRumbleTriggers(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickHasRumbleTriggers");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GameControllerHasRumble(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasRumble");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_GameControllerHasRumbleTriggers(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerHasRumbleTriggers");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
void abi_SDL_hid_ble_scan(SDL_bool a)
{
    void *addr = cosmo_dlsym(libD, "SDL_hid_ble_scan");
    if (!IsWindows()) {
        void (*local)(SDL_bool a) = addr;
        local(a);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_bool a) = addr;
        local(a);
    }
}
int abi_SDL_PremultiplyAlpha(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h)
{
    void *addr = cosmo_dlsym(libD, "SDL_PremultiplyAlpha");
    if (!IsWindows()) {
        int (*local)(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    } else {
        int __attribute__((__ms_abi__)) (*local)(int a, int b, Uint32 c, const void *d, int e, Uint32 f, void *g, int h) = addr;
        return local(a, b, c, d, e, f, g, h);
    }
}
const char *abi_SDL_GetTouchName(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetTouchName");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
void abi_SDL_ClearComposition(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ClearComposition");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
SDL_bool abi_SDL_IsTextInputShown(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_IsTextInputShown");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasIntersectionF(const SDL_FRect *a, const SDL_FRect *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasIntersectionF");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_FRect *a, const SDL_FRect *b) = addr;
        return local(a, b);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_FRect *a, const SDL_FRect *b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_IntersectFRect(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_IntersectFRect");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c) = addr;
        return local(a, b, c);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_UnionFRect(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_UnionFRect");
    if (!IsWindows()) {
        void (*local)(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *c) = addr;
        local(a, b, c);
    }
}
SDL_bool abi_SDL_EncloseFPoints(const SDL_FPoint *a, int b, const SDL_FRect *c, SDL_FRect *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_EncloseFPoints");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_FPoint *a, int b, const SDL_FRect *c, SDL_FRect *d) = addr;
        return local(a, b, c, d);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_FPoint *a, int b, const SDL_FRect *c, SDL_FRect *d) = addr;
        return local(a, b, c, d);
    }
}
SDL_bool abi_SDL_IntersectFRectAndLine(const SDL_FRect *a, float *b, float *c, float *d, float *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_IntersectFRectAndLine");
    if (!IsWindows()) {
        SDL_bool (*local)(const SDL_FRect *a, float *b, float *c, float *d, float *e) = addr;
        return local(a, b, c, d, e);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const SDL_FRect *a, float *b, float *c, float *d, float *e) = addr;
        return local(a, b, c, d, e);
    }
}
SDL_Window *abi_SDL_RenderGetWindow(SDL_Renderer *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_RenderGetWindow");
    if (!IsWindows()) {
        SDL_Window *(*local)(SDL_Renderer * a) = addr;
        return local(a);
    } else {
        SDL_Window *__attribute__((__ms_abi__)) (*local)(SDL_Renderer * a) = addr;
        return local(a);
    }
}
void *abi_SDL_bsearch(const void *a, const void *b, size_t c, size_t d, int(SDLCALL *e)(const void *, const void *))
{
    void *addr = cosmo_dlsym(libD, "SDL_bsearch");
    if (!IsWindows()) {
        void *(*local)(const void *a, const void *b, size_t c, size_t d, int(SDLCALL * e)(const void *, const void *)) = addr;
        return local(a, b, c, d, e);
    } else {
        void *__attribute__((__ms_abi__)) (*local)(const void *a, const void *b, size_t c, size_t d, int(SDLCALL * e)(const void *, const void *)) = addr;
        return local(a, b, c, d, e);
    }
}
const char *abi_SDL_GameControllerPathForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerPathForIndex");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_GameControllerPath(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerPath");
    if (!IsWindows()) {
        const char *(*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
const char *abi_SDL_JoystickPathForIndex(int a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickPathForIndex");
    if (!IsWindows()) {
        const char *(*local)(int a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(int a) = addr;
        return local(a);
    }
}
const char *abi_SDL_JoystickPath(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickPath");
    if (!IsWindows()) {
        const char *(*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        const char *__attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
int abi_SDL_JoystickAttachVirtualEx(const SDL_VirtualJoystickDesc *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickAttachVirtualEx");
    if (!IsWindows()) {
        int (*local)(const SDL_VirtualJoystickDesc *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const SDL_VirtualJoystickDesc *a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_GameControllerGetFirmwareVersion(SDL_GameController *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetFirmwareVersion");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_GameController * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_GameController * a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_JoystickGetFirmwareVersion(SDL_Joystick *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_JoystickGetFirmwareVersion");
    if (!IsWindows()) {
        Uint16 (*local)(SDL_Joystick * a) = addr;
        return local(a);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(SDL_Joystick * a) = addr;
        return local(a);
    }
}
void abi_SDL_GUIDToString(SDL_GUID a, char *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GUIDToString");
    if (!IsWindows()) {
        void (*local)(SDL_GUID a, char *b, int c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_GUID a, char *b, int c) = addr;
        local(a, b, c);
    }
}
SDL_GUID abi_SDL_GUIDFromString(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GUIDFromString");
    if (!IsWindows()) {
        SDL_GUID (*local)
        (const char *a) = addr;
        return local(a);
    } else {
        SDL_GUID __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_HasLSX(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasLSX");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasLASX(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasLASX");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
size_t abi_SDL_utf8strnlen(const char *a, size_t b)
{
    void *addr = cosmo_dlsym(libD, "SDL_utf8strnlen");
    if (!IsWindows()) {
        size_t (*local)(const char *a, size_t b) = addr;
        return local(a, b);
    } else {
        size_t __attribute__((__ms_abi__)) (*local)(const char *a, size_t b) = addr;
        return local(a, b);
    }
}
void abi_SDL_GetOriginalMemoryFunctions(SDL_malloc_func *a, SDL_calloc_func *b, SDL_realloc_func *c, SDL_free_func *d)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetOriginalMemoryFunctions");
    if (!IsWindows()) {
        void (*local)(SDL_malloc_func * a, SDL_calloc_func * b, SDL_realloc_func * c, SDL_free_func * d) = addr;
        local(a, b, c, d);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_malloc_func * a, SDL_calloc_func * b, SDL_realloc_func * c, SDL_free_func * d) = addr;
        local(a, b, c, d);
    }
}
void abi_SDL_ResetKeyboard(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ResetKeyboard");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
int abi_SDL_GetDefaultAudioInfo(char **a, SDL_AudioSpec *b, int c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetDefaultAudioInfo");
    if (!IsWindows()) {
        int (*local)(char **a, SDL_AudioSpec *b, int c) = addr;
        return local(a, b, c);
    } else {
        int __attribute__((__ms_abi__)) (*local)(char **a, SDL_AudioSpec *b, int c) = addr;
        return local(a, b, c);
    }
}
int abi_SDL_GetPointDisplayIndex(const SDL_Point *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPointDisplayIndex");
    if (!IsWindows()) {
        int (*local)(const SDL_Point *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const SDL_Point *a) = addr;
        return local(a);
    }
}
int abi_SDL_GetRectDisplayIndex(const SDL_Rect *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetRectDisplayIndex");
    if (!IsWindows()) {
        int (*local)(const SDL_Rect *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const SDL_Rect *a) = addr;
        return local(a);
    }
}
SDL_bool abi_SDL_ResetHint(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_ResetHint");
    if (!IsWindows()) {
        SDL_bool (*local)(const char *a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
Uint16 abi_SDL_crc16(Uint16 a, const void *b, size_t c)
{
    void *addr = cosmo_dlsym(libD, "SDL_crc16");
    if (!IsWindows()) {
        Uint16 (*local)(Uint16 a, const void *b, size_t c) = addr;
        return local(a, b, c);
    } else {
        Uint16 __attribute__((__ms_abi__)) (*local)(Uint16 a, const void *b, size_t c) = addr;
        return local(a, b, c);
    }
}
void abi_SDL_GetWindowSizeInPixels(SDL_Window *a, int *b, int *c)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetWindowSizeInPixels");
    if (!IsWindows()) {
        void (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_Window * a, int *b, int *c) = addr;
        local(a, b, c);
    }
}
void abi_SDL_GetJoystickGUIDInfo(SDL_JoystickGUID a, Uint16 *b, Uint16 *c, Uint16 *d, Uint16 *e)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetJoystickGUIDInfo");
    if (!IsWindows()) {
        void (*local)(SDL_JoystickGUID a, Uint16 * b, Uint16 * c, Uint16 * d, Uint16 * e) = addr;
        local(a, b, c, d, e);
    } else {
        void __attribute__((__ms_abi__)) (*local)(SDL_JoystickGUID a, Uint16 * b, Uint16 * c, Uint16 * d, Uint16 * e) = addr;
        local(a, b, c, d, e);
    }
}
int abi_SDL_SetPrimarySelectionText(const char *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_SetPrimarySelectionText");
    if (!IsWindows()) {
        int (*local)(const char *a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(const char *a) = addr;
        return local(a);
    }
}
char *abi_SDL_GetPrimarySelectionText(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_GetPrimarySelectionText");
    if (!IsWindows()) {
        char *(*local)(void) = addr;
        return local();
    } else {
        char *__attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
SDL_bool abi_SDL_HasPrimarySelectionText(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasPrimarySelectionText");
    if (!IsWindows()) {
        SDL_bool (*local)(void) = addr;
        return local();
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(void) = addr;
        return local();
    }
}
int abi_SDL_GameControllerGetSensorDataWithTimestamp(SDL_GameController *a, SDL_SensorType b, Uint64 *c, float *d, int e)
{
    void *addr = cosmo_dlsym(libD, "SDL_GameControllerGetSensorDataWithTimestamp");
    if (!IsWindows()) {
        int (*local)(SDL_GameController * a, SDL_SensorType b, Uint64 * c, float *d, int e) = addr;
        return local(a, b, c, d, e);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_GameController * a, SDL_SensorType b, Uint64 * c, float *d, int e) = addr;
        return local(a, b, c, d, e);
    }
}
int abi_SDL_SensorGetDataWithTimestamp(SDL_Sensor *a, Uint64 *b, float *c, int d)
{
    void *addr = cosmo_dlsym(libD, "SDL_SensorGetDataWithTimestamp");
    if (!IsWindows()) {
        int (*local)(SDL_Sensor * a, Uint64 * b, float *c, int d) = addr;
        return local(a, b, c, d);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Sensor * a, Uint64 * b, float *c, int d) = addr;
        return local(a, b, c, d);
    }
}
void abi_SDL_ResetHints(void)
{
    void *addr = cosmo_dlsym(libD, "SDL_ResetHints");
    if (!IsWindows()) {
        void (*local)(void) = addr;
        local();
    } else {
        void __attribute__((__ms_abi__)) (*local)(void) = addr;
        local();
    }
}
char *abi_SDL_strcasestr(const char *a, const char *b)
{
    void *addr = cosmo_dlsym(libD, "SDL_strcasestr");
    if (!IsWindows()) {
        char *(*local)(const char *a, const char *b) = addr;
        return local(a, b);
    } else {
        char *__attribute__((__ms_abi__)) (*local)(const char *a, const char *b) = addr;
        return local(a, b);
    }
}
SDL_bool abi_SDL_HasWindowSurface(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_HasWindowSurface");
    if (!IsWindows()) {
        SDL_bool (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        SDL_bool __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
int abi_SDL_DestroyWindowSurface(SDL_Window *a)
{
    void *addr = cosmo_dlsym(libD, "SDL_DestroyWindowSurface");
    if (!IsWindows()) {
        int (*local)(SDL_Window * a) = addr;
        return local(a);
    } else {
        int __attribute__((__ms_abi__)) (*local)(SDL_Window * a) = addr;
        return local(a);
    }
}
