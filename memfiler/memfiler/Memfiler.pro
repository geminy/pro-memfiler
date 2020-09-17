TARGET = memfiler
TEMPLATE = lib

##################################################

include($$PWD/../MF.pri)
DESTDIR = $$DESTDIR_LIB

QMAKE_CFLAGS += -g
QMAKE_CXXFLAGS += -g
QMAKE_CFLAGS += -pthread
QMAKE_CXXFLAGS += -pthread
QMAKE_LFLAGS += -pthread
QMAKE_LIBS += -ldl
#QMAKE_LIBS += -lrt
#QMAKE_LIBS += -lunwind

##################################################

MF_HOOK_C = "TRUE"
equals(MF_HOOK_C, "TRUE") {
    message("malloc hook is available.")
    DEFINES += MF_HOOK_C
} else {
    message("malloc hook is forbidden.")
}

MF_HOOK_CXX = "FALSE"
equals(MF_HOOK_CXX, "TRUE") {
    message("new hook is available.")
    DEFINES += MF_HOOK_CXX
} else {
    message("new hook is forbidden.")
}

##################################################

#MF_USE_WRAP = "TRUE"
MF_USE_WRAP = "FALSE"
equals(MF_USE_WRAP, "TRUE") {
    equals(MF_HOOK_C, "TRUE") {
        error("wrap and malloc hook cannot be both available at the same time.")
    }
    equals(MF_HOOK_CXX, "TRUE") {
        error("wrap and new hook cannot be both available at the same time.")
    }
    message("wrap is available.")
    DEFINES += MF_USE_WRAP
    QMAKE_LFLAGS += -Wl,--wrap,malloc
    QMAKE_LFLAGS += -Wl,--wrap,free
    QMAKE_LFLAGS += -Wl,--wrap,calloc
    QMAKE_LFLAGS += -Wl,--wrap,realloc
    QMAKE_LFLAGS += -Wl,--wrap,memalign
} else {
    message("wrap is forbidden.")
}

##################################################

#MF_USE_HOOK = "TRUE"
MF_USE_HOOK = "FALSE"
equals(MF_USE_HOOK, "TRUE") {
    equals(MF_HOOK_C, "TRUE") {
        error("hook and malloc hook cannot be both available at the same time.")
    }
    equals(MF_HOOK_CXX, "TRUE") {
        error("hook and new hook cannot be both available at the same time.")
    }
    message("hook is available.")
    DEFINES += MF_USE_HOOK
} else {
    message("hook is forbidden.")
}

equals(MF_USE_WRAP, "TRUE") {
    equals(MF_USE_HOOK, "TRUE") {
        error("wrap and hook cannot be both available at the same time.")
    }
}

##################################################

HEADERS += \
    Memfiler.h \
    base/MFBase.h \
    base/MFMutex.h \
    base/MFSystemHandle.h \
    malloc/MFWorker.h \
    malloc/MFCheck.h \
    backtrace/MFBacktrace.h

SOURCES += \
    Memfiler.cpp \
    base/MFMutex.cpp \
    base/MFSystemHandle.cpp \
    malloc/MFMalloc.cpp \
    malloc/MFWorker.cpp \
    backtrace/MFBacktrace.cpp \
    misc/Assistant.cpp \
    misc/Wrapper.cpp \
    misc/Hooker.cpp
