/**
 * gcc attribute:
 *     __attribute__((constructor))
 *     __attribute__((destructor))
 */

#include "Global.h"
#include "Memfiler.h"

MF_DECLS void MFMallocHookInit();
MF_DECLS void MFMallocHookDnit();

/**************************************************/

static void MF_CONSTRUCTOR _MFInitialize()
{
    logf();

    Memfiler::InitializeMF();

#ifdef MF_USE_HOOK
    MFMallocHookInit();
#endif
}

static void MF_DESTRUCTOR _MFFinalize()
{
    logf();

#ifdef MF_USE_HOOK
    MFMallocHookDnit();
#endif
}
