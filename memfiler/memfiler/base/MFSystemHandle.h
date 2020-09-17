/**
 * @file MFSystemHandle.h
 * @brief System handle
 * @note cpp
 */

#ifndef MF_MFSYSTEMHANDLE_H
#define MF_MFSYSTEMHANDLE_H

#ifndef __cplusplus
# error ERROR: Must be included at .cpp file
#endif

/**
 * @class MFSystemHandle
 * @brief System handle
 */
class MFSystemHandle
{
public:
    MFSystemHandle();
    ~MFSystemHandle();

    void handle();

private:
    void *m_libc;
};

#endif // MF_MFSYSTEMHANDLE_H
