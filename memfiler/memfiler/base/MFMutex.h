/**
 * @file MFMutex.h
 * @brief Mutex definitions
 * @note cpp
 */

#ifndef MF_MFMUTEX_H
#define MF_MFMUTEX_H

#ifndef __cplusplus
# error ERROR: Must be included at .cpp file
#endif

#include <pthread.h>

/**
 * @class MFMutex
 * @brief Mutex definitions
 */
class MFMutex
{
public:
    /**
     * @enum MFMutexAttr
     * @brief Mutex attribution
     */
    enum MFMutexAttr
    {
        PRIVATE,
        SHARED
    };

    MFMutex();
    explicit MFMutex(MFMutexAttr type);
    ~MFMutex();

    void lock();
    void unlock();

    /**
     * @class MFAutoLock
     * @brief Auto lock definitions
     */
    class MFAutoLock
    {
    public:
        explicit MFAutoLock(MFMutex &mutex)
            : m_lock(mutex)
        {
            m_lock.lock();
        }

        explicit MFAutoLock(MFMutex *mutex)
            : m_lock(*mutex)
        {
            m_lock.lock();
        }

        ~MFAutoLock()
        {
            m_lock.unlock();
        }

    private:
        MFMutex &m_lock;
    };

private:
    pthread_mutex_t m_mutex;
};

/**************************************************/

typedef MFMutex::MFAutoLock MFAutoMutex;

#endif // MF_MFMUTEX_H
