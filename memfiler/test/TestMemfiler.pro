TARGET = testmemfiler
TEMPLATE = app

include($$PWD/../MF.pri)
DESTDIR = $$DESTDIR_BIN
QMAKE_CFLAGS += -g
QMAKE_CXXFLAGS += -g
LIBS += -lmemfiler

HEADERS += \
    TestSuite.h \
    TestMemfiler.h

SOURCES += \
    TestMemfiler.cpp \
    TestMalloc.c
