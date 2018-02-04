#ifndef MOXIE_SYSTEMHOOK_H
#define MOXIE_SYSTEMHOOK_H

using read_unix = ssize_t (*)(int fildes, void *buf, size_t nbyte);
using write_unix = ssize_t (*)(int fd, const void *buf, size_t count);
using close_unix = int (*)(int fd);
using socket_unix = int (*)(int domain, int type, int protocol);
using fcntl_unix = int (*)(int fildes, int cmd, ...);

extern read_unix read_uni_func;
extern write_unix write_uni_func;
extern close_unix close_uni_func;
extern socket_unix socket_uni_func;
extern fcntl_unix fcntl_uni_func;

#endif //MOXIE_SYSTEMHOOK_H
