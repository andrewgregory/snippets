#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

FILE *fopenat(int dirfd, const char *path, const char *mode) {
    int fd, flags = 0, rwflag = 0;
    FILE *stream;
    switch(*(mode++)) {
        case 'r': rwflag = O_RDONLY; break;
        case 'w': rwflag = O_WRONLY; flags |= O_CREAT | O_TRUNC; break;
        case 'a': rwflag = O_WRONLY; flags |= O_CREAT | O_APPEND; break;
        default: errno = EINVAL; return NULL;
    }
    if(mode[1] == 'b') { mode++; }
    if(mode[1] == '+') { mode++; rwflag = O_RDWR; }
    while(*mode) {
        switch(*(mode++)) {
            case 'e': flags |= O_CLOEXEC; break;
            case 'x': flags |= O_EXCL; break;
        }
    }
    if((fd = openat(dirfd, path, flags | rwflag), 0666) < 0) { return NULL; }
    if((stream = fdopen(fd, mode)) == NULL) { close(fd); return NULL; }
    return stream;
}

FILE *fopenat2(int dirfd, const char *path, const char *mode) {
    if(dirfd == AT_FDCWD) {
        return fopen(path, mode);
    } else {
        FILE *f;
        int cwd = open(".", O_DIRECTORY);
        if(cwd < 0 || fchdir(dirfd) != 0) { return NULL; }
        f = fopen(path, mode);
        fchdir(cwd);
        close(cwd);
        return f;
    }
}
