##################################################
# Project: MF - Memory Profiler                                                        #
##################################################

TEMPLATE = subdirs

##################################################

# memfiler
SUBDIRS += memfiler
memfiler.file = memfiler/Memfiler.pro

##################################################

# test
SUBDIRS += test
test.file = test/TestMemfiler.pro
test.depends += memfiler

##################################################

HEADERS += \
    include/Memfiler.h \
    include/Global.h

##################################################

OTHER_FILES += \
    tool/foremost.sh \
    tool/memfiler.py
