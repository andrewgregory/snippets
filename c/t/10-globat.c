#define _GNU_SOURCE
#include <stdarg.h>
#include <stdlib.h>

#include "tap/tap.c"

#include "../globat.c"

#define IS(g, ...) is(g, #g, __LINE__, __VA_ARGS__)

void is(char **got, char *msg, int line, ...) {
    char *e, **g = got;
    va_list ap, a;
    va_start(ap, line);

    va_copy(a, ap);

    e = va_arg(a, char*);
    while(*g && e) {
        if(strcmp(*g, e) != 0) { break; }
        g++;
        e = va_arg(a, char*);
    }
    va_end(a);

    if(*g != NULL || e != NULL) {
        g = got;
        _tap_ok(__FILE__, line, 0, msg);
        tap_diag("         got:");
        while(*g) { tap_diag("            : '%s'", *g++); }
        tap_diag("    expected:");
        while((e = va_arg(ap, char*))) { tap_diag("            : '%s'", e); }
    } else {
        _tap_ok(__FILE__, line, 1, msg);
    }
    _glob_freepattern(got);
}

int main(void) {
    tap_plan(11);

    IS(_glob_split_pattern2("foo/bar"), "foo", "bar", NULL);
    IS(_glob_split_pattern2("foo/bar/"), "foo", "bar", "/", NULL);
    IS(_glob_split_pattern2("//foo//bar//"), "/", "foo", "bar", "/", NULL);

    return tap_finish();
}
