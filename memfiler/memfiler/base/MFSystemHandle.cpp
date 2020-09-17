#include "MFSystemHandle.h"

#include <cstddef>
#include <cassert>
#include <dlfcn.h>

void*(*__mf__libc_memalign)(size_t, size_t) = NULL;

/**************************************************/

MFSystemHandle::MFSystemHandle()
    : m_libc(NULL)
{
}

MFSystemHandle::~MFSystemHandle()
{
    if (NULL != m_libc) {
        dlclose(m_libc);
        m_libc = NULL;
    }
}

void MFSystemHandle::handle()
{
    if (NULL == m_libc) {
        m_libc = dlopen("libc.so.6", RTLD_LAZY);
    }
    assert(NULL != m_libc);

    if (NULL == __mf__libc_memalign) {
        __mf__libc_memalign = ((void*(*)(size_t, size_t))dlsym(m_libc, "__libc_memalign"));
    }
    assert(NULL != __mf__libc_memalign);
}
