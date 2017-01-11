AUTOMAKE_OPTIONS = foreign #include-deps
where_PRIMARY =  _LTLIBRARIES 

lib_LTLIBRARIES=libconcurrent.la

libconcurrent_la_SOURCES= \
    concurrentfactory.h \
    concurrent.h \
    concurrentpool.h \
    concurrentqueue.h \
    concurrent_util.h \
    concurrentworker.h \
    hashmap.h \
    log.h \
    slinkedlist.h \
    stdlib_util.h \
    time_util.h \
    concurrent.c \
    concurrentfactory.c \
    concurrentpool.c \
    concurrentqueue.c \
    concurrent_util.c \
    concurrentworker.c \
    hashmap.c \
    log.c \
    slinkedlist.c \
    stdlib_util.c \
    time_util.c

libconcurrent_la_HEADERS= \
    concurrent.h \
    concurrentpool.h \
    concurrentqueue.h \
    concurrent_util.h \
    concurrentworker.h

libconcurrent_ladir=$(includedir)

libconcurrent_la_LDFLAGS=@LDFLAGS@ -version-info 1:0:0

include_HEADERS= \
    concurrent.h \
    concurrentpool.h \
    concurrentqueue.h \
    concurrent_util.h \
    concurrentworker.h
