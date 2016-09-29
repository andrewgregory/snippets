
#include "tap/tap.c"

#include "../capture.c"

typedef void (*test_func)(void);

void caller(void *f) { ((test_func)f)(); }

void test1(void) {
    fputs("line1\n", stdout);
    fputs("line2\n", stderr);
}

int main(void) {
    char *out = NULL;

    tap_plan(2);

    tap_is_int(capture(caller, test1, &out), 0, NULL);
    tap_is_str(out, "line1\nline2\n", NULL);

    free(out);

    return tap_finish();
}
