#include "Global.h"
#include "base/MFBase.h"
#include "Memfiler.h"

#include <new>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

Memfiler::Memfiler()
{
//    initialize(); // can not called here
}

Memfiler::~Memfiler()
{
}

MFMallocWorker* Memfiler::mallocWorker()
{
    return &m_malloc;
}

Memfiler* Memfiler::m_inst = NULL;
bool Memfiler::m_initialized = false;

Memfiler* Memfiler::MF()
{
    if (NULL == m_inst) {
        int len = sizeof(Memfiler);
        void *p = __libc_malloc(len);
        m_inst = new(p)Memfiler();
        m_inst->initialize();
        logp("Memfiler::MF created addr=%p len=%d\n", m_inst, len);
    }
    return m_inst;
}

bool Memfiler::InitializeMF()
{
    if (m_initialized) {
        return false;
    }

    pthread_t commu;
    int state = pthread_create(&commu, NULL, MFThreadRoutine, NULL);
    if (0 != state) {
        logp("@Memfiler::InitializeMF pthread_create failed %s\n", strerror(state));
        abort();
    }

    logp("Memfiler::InitializeMF initialized\n");
    m_initialized = true;
    return true;
}

void Memfiler::initialize()
{
    m_handle.handle();
}

void* Memfiler::MFThreadRoutine(void*)
{
    logp("Memfiler::MFThreadRoutine entered\n");

    char path[MF_FIFO_PATHSIZE] = { 0 };
    sprintf(path, "%s%d%s", MF_FIFO_PREFIX, getpid(), MF_FIFO_SUFFIX);

    if (mkfifo(path, MF_FIFO_MODE) == -1 && EEXIST != errno) {
        logp("@Memfiler::MFThreadRoutine mkfifo failed %s\n", strerror(errno));
        abort();
    }

    int fd = open(path, O_RDONLY);
    if (-1 == fd) {
        logp("@Memfiler::MFThreadRoutine open failed %s\n", strerror(errno));
        abort();
    }

    char cmd[MF_FIFO_CMDSIZE] = { 0 };
    register int len;
    while (true) {
        memset(cmd, 0, sizeof(cmd));
        len = read(fd, cmd, sizeof(cmd) - 1);
        if (-1 == len) {
            logp("@Memfiler::MFThreadRoutine read failed %s\n", strerror(errno));
            break;
        }
        else if (0 < len) {
            if (!MFCmdProc(cmd, len)) {
                break;
            }
        }
    }

    close(fd);
    unlink(path);

    logp("Memfiler::MFThreadRoutine exitd\n");

    return NULL;
}

bool Memfiler::MFCmdProc(char *cmd, int len)
{
    if (strncmp(cmd + len - 1, "\n", 1) == 0) {
        cmd[--len] = 0;
    }
    logp("#Memfiler::MFCmdProc cmd %s %d\n", cmd, len);

    if (strncmp(MF_FIFO_QUIT, cmd, strlen(MF_FIFO_QUIT)) == 0) {
        return false;
    }
    else if (strncmp(MF_FIFO_DUMP, cmd, strlen(MF_FIFO_DUMP)) == 0) {
        _mf->mallocWorker()->dumpAll();
    }
    else if (strncmp(MF_FIFO_CHECK, cmd, strlen(MF_FIFO_CHECK)) == 0) {
        _mf->mallocWorker()->checkAllBlock();
    }

    return true;
}
