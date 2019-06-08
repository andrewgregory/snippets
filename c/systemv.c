#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>

extern char **environ;

int systemvp(const char *file, char *const argv[]) {
    int *err = mmap(NULL, sizeof(*err), PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int pid = fork();

    if(pid == -1) {
        /* error */
        munmap(err, sizeof(*err));
        return -1;
    } else if(pid == 0) {
        /* child */
        *err = 0;
        execvp(file, argv);
        *err = errno;
        munmap(err, sizeof(*err));
        _Exit(1);
    } else {
        /* parent */
        int status;
        while(waitpid(pid, &status, 0) == -1) {
            if(errno != EINTR) {
                munmap(err, sizeof(*err));
                return -1;
            }
        }
        if(*err != 0) {
            errno = *err;
            status = -1;
        }
        munmap(err, sizeof(*err));
        return status;
    }
}

int systemve(const char *path, char *const argv[], char *const envp[]) {
    int *err = mmap(NULL, sizeof(*err), PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int pid = fork();

    if(pid == -1) {
        /* error */
        munmap(err, sizeof(*err));
        return -1;
    } else if(pid == 0) {
        /* child */
        *err = 0;
        execve(path, argv, envp);
        *err = errno;
        munmap(err, sizeof(*err));
        _Exit(1);
    } else {
        /* parent */
        int status;
        while(waitpid(pid, &status, 0) == -1) {
            if(errno != EINTR) {
                munmap(err, sizeof(*err));
                return -1;
            }
        }
        if(*err != 0) {
            errno = *err;
            status = -1;
        }
        munmap(err, sizeof(*err));
        return status;
    }
}

int systemv(const char *path, char *const argv[]) {
    return systemve(path, argv, environ);
}

int systeml(const char *path, ...) {
    char **argv;
    va_list ap, cp;
    int count, i, ret;

    /* count args */
    count = 0;
    va_start(cp, path);
    while(va_arg(cp, char*) != NULL) {
        count++;
    }
    va_end(cp);

    if((argv = calloc(count + 1, sizeof(char*))) == NULL) {
        errno = ENOMEM;
        return -1;
    }

    /* copy args */
    i = 0;
    va_start(ap, path);
    while(i < count) {
        argv[i++] = va_arg(ap, char*);
    }
    va_end(ap);

    ret = systemv(path, argv);
    free(argv);

    return ret;
}

int systemle(const char *path, ...) {
    char **argv, **envp;
    va_list ap, cp;
    int count, i, ret;

    /* count args */
    count = 0;
    va_start(cp, path);
    while(va_arg(cp, char*) != NULL) {
        count++;
    }
    va_end(cp);

    if((argv = calloc(count + 1, sizeof(char*))) == NULL) {
        errno = ENOMEM;
        return -1;
    }

    /* copy args */
    count = 0;
    va_start(ap, path);
    while(i < count) {
        argv[i++] = va_arg(ap, char*);
    }
    envp = va_arg(ap, char**);
    va_end(ap);

    ret = systemve(path, argv, envp);
    free(argv);

    return ret;
}
