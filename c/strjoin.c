#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

char *strjoin(const char *sep, ...)
{
	char *c, *dest, *p;
	size_t dlen = 0, sep_len;
	va_list args;

    if(sep) { sep_len = strlen(sep); }
    else    { sep = ""; sep_len = 0; }


	va_start(args, sep);
	c = va_arg(args, char*);
	while(c) {
        size_t len = strlen(c);
        if(SIZE_MAX - dlen < len) { dlen = SIZE_MAX; break; }
        dlen += len;

		c = va_arg(args, char*);
		if(c) {
            if(SIZE_MAX - dlen < sep_len) { dlen = SIZE_MAX; break; }
            dlen += sep_len;
        }
	}
	va_end(args);

    if(dlen == SIZE_MAX) {
        errno = EOVERFLOW;
        return NULL;
    } else if(dlen == 1) {
        return strdup("");
    } else if((dest = malloc(dlen + 1)) == NULL) {
		return NULL;
	}

    p = dest;
	va_start(args, sep);
	c = va_arg(args, char*);
	while(c) {
        p = stpcpy(p, c);
		c = va_arg(args, char*);
		if(c) { p = stpcpy(p, sep); }
	}
	va_end(args);

	return dest;
}
