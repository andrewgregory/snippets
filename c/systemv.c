#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

/* system()/exec() hybrid functions allowing exec()-style direct execution
 * of a command with the simplicity of system()
 * - not thread-safe
 * - errno may be set by fork(), pipe(), or execve()/execvp()
 */

int systemvp(const char *file, char *const argv[]) {
    int pid, err = 0, ret = -1, err_fd[2];
    sigset_t oldblock;
    struct sigaction sa_ign = { .sa_handler = SIG_IGN }, oldint, oldquit;

    if(pipe(err_fd) != 0) {
        return -1;
    }

    sigaction(SIGINT, &sa_ign, &oldint);
    sigaction(SIGQUIT, &sa_ign, &oldquit);
    sigaddset(&sa_ign.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa_ign.sa_mask, &oldblock);

    pid = fork();

    /* child */
    if(pid == 0) {
        close(err_fd[0]);
        fcntl(err_fd[1], F_SETFD, FD_CLOEXEC);

        /* restore signal handling for the child to inherit */
        sigaction(SIGINT, &oldint, NULL);
        sigaction(SIGQUIT, &oldquit, NULL);
        sigprocmask(SIG_SETMASK, &oldblock, NULL);

        execvp(file, argv);

        /* execvp failed, pass the error back to the parent */
        while(write(err_fd[1], &errno, sizeof(errno)) == -1 && errno == EINTR);
        _Exit(127);
    }

    /* parent */
    close(err_fd[1]);

    if(pid != -1)  {
        int wret;
        while((wret = waitpid(pid, &ret, 0)) == -1 && errno == EINTR);
        if(wret > 0) {
            while(read(err_fd[0], &err, sizeof(err)) == -1 && errno == EINTR);
        }
    } else {
        /* fork failed, make sure errno is preserved after cleanup */
        err = errno;
    }

    close(err_fd[0]);

    sigaction(SIGINT, &oldint, NULL);
    sigaction(SIGQUIT, &oldquit, NULL);
    sigprocmask(SIG_SETMASK, &oldblock, NULL);

    if(err) {
        errno = err;
        ret = -1;
    }

    return ret;
}

int systemve(const char *path, char *const argv[], char *const envp[]) {
    int pid, err = 0, ret = -1, err_fd[2];
    sigset_t oldblock;
    struct sigaction sa_ign = { .sa_handler = SIG_IGN }, oldint, oldquit;

    if(pipe(err_fd) != 0) {
        return -1;
    }

    sigaction(SIGINT, &sa_ign, &oldint);
    sigaction(SIGQUIT, &sa_ign, &oldquit);
    sigaddset(&sa_ign.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa_ign.sa_mask, &oldblock);

    pid = fork();

    /* child */
    if(pid == 0) {
        close(err_fd[0]);
        fcntl(err_fd[1], F_SETFL, FD_CLOEXEC);

        sigaction(SIGINT, &oldint, NULL);
        sigaction(SIGQUIT, &oldquit, NULL);
        sigprocmask(SIG_SETMASK, &oldblock, NULL);

        execve(path, argv, envp);

        write(err_fd[1], &errno, sizeof(errno));
        _Exit(127);
    }

    /* parent */
    close(err_fd[1]);

    if(pid != -1)  {
        int wret;
        while((wret = waitpid(pid, &ret, 0)) == -1 && errno == EINTR);
        if(wret > 0 && WEXITSTATUS(ret) == 127) {
            read(err_fd[0], &err, sizeof(err));
        }
    } else {
        /* fork failed, make sure errno is preserved after cleanup */
        err = errno;
    }

    close(err_fd[0]);

    sigaction(SIGINT, &oldint, NULL);
    sigaction(SIGQUIT, &oldquit, NULL);
    sigprocmask(SIG_SETMASK, &oldblock, NULL);

    if(err) {
        errno = err;
        ret = -1;
    }

    return ret;
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
    i = 0;
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
