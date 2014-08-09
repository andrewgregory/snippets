#include <errno.h>
#include <stdio.h>

/* signal handlers interrupt primitives like read(2) causing stdio functions to
 * fail even though no actual error has occurred. */

/* fgetc wrapper that retries on EINTR */
int efgetc(FILE *stream) {
    int c, errno_orig = errno;
    do { errno = 0; c = fgetc(stream); } while(c == EOF && errno == EINTR);
    if(c != EOF || feof(stream) ) { errno = errno_orig; }
    return c;
}

/* fgets wrapper that retries on EINTER */
/* depending on how the underlying libc handles reading,
 * this *should* be safe, but could return partial lines */
char *efgets(char *buf, int size, FILE *stream) {
    char *ret;
    int errno_orig = errno_orig;
    do {
        errno = 0;
        ret = fgets(buf, size, stream);
    } while(ret == NULL && errno == EINTR);
    if(ret != NULL || feof(stream)) { errno = errno_orig; }
    return ret;
}

/* fgets replacement to handle signal interrupts */
/* this should almost certainly be safe even if the
 * underlying libc is naive about how it performs reads */
char *efgets(char *buf, int size, FILE *stream) {
    char *s = buf;
    int c;

    if(size == 1) {
        *s = '\0';
        return buf;
    } else if(size < 1) {
        return NULL;
    }

    do {
        c = efgetc(stream);
        if(c != EOF) { *(s++) = (char) c; }
    } while(--size > 1 && c != EOF && c != '\n');

    if(s == buf || ( c == EOF && !feof(stream) )) {
        return NULL;
    } else {
        *s = '\0';
        return buf;
    }
}
