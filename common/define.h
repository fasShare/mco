#ifndef DEFINE_H
#define DEFINE_H
#include <unistd.h>
#include <sys/syscall.h>

#define gettid() (::syscall(SYS_gettid))

#endif
