#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

char *vasprintf(const char *fmt, va_list args) {
    va_list args_copy;
    char *p;
    int len;

    va_copy(args_copy, args);
    len = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

#if SIZE_MAX <= INT_MAX
    if(len >= SIZE_MAX) {
        errno = EOVERFLOW;
        return NULL;
    }
#endif

    if(len < 0) {
        return NULL;
    }

    if((p = malloc((size_t)len + 1)) == NULL) {
        return NULL;
    }

    vsprintf(p, fmt, args);

    return p;
}

char *asprintf(const char *fmt, ...) {
    char *p;
    va_list args;

    va_start(args, fmt);
    p = vasprintf(fmt, args);
    va_end(args);

    return p;
}
