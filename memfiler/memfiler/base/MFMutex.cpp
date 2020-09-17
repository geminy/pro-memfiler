#include "MFMutex.h"

MFMutex::MFMutex()
{
    pthread_mutex_init(&m_mutex, NULL);
}

MFMutex::MFMutex(MFMutexAttr type)
{
    if (SHARED == type) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&m_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    else {
        pthread_mutex_init(&m_mutex, NULL);
    }
}

MFMutex::~MFMutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void MFMutex::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void MFMutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}
