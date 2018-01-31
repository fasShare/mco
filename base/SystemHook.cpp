#include <unistd.h>
#include <dlfcn.h>

#include <SystemHook.h>
#include <define.h>

read_unix read_uni_func = (read_unix)dlsym(RTLD_NEXT, "read");
write_unix write_uni_func = (write_unix)dlsym(RTLD_NEXT, "write");
close_unix close_uni_func = (close_unix)dlsym(RTLD_NEXT, "close");
socket_unix socket_uni_func = (socket_unix)dlsym(RTLD_NEXT, "socket");
fcntl_unix fcntl_uni_func = (fcntl_unix)dlsym(RTLD_NEXT, "fcntl");

ssize_t read(int fd, void *buf, size_t nbyte) {
#ifdef GETLOOPNOSAFE
    auto loop = EventLoopPool::GetLoopNosafe(gettid());
#else
    auto loop = EventLoopPool::GetLoop(gettid());
#endif
    if (!loop) {
        return read_uni_func(fd, buf, nbyte);
    }
    auto ectx = loop->econtext(fd);
    if (!ectx) {
        return read_uni_func(fd, buf, nbyte);
    }



    return 0;
}

ssize_t write(int fd, const void *buf, size_t count) {
    return 0;
}

int close(int fd) {
    return 0;
}

int socket(int domain, int type, int protocol) {
    return 0;
}

int fcntl(int fildes, int cmd, ...) {
    return 0;
}
