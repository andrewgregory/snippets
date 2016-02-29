#include "tap/tap.c"
#include "../pathcmp.c"

#define is(g, e) tap_is_int(g, e, "%s == %s", #g, #e)
#define ok(r) tap_ok(r, "%s", #r)

static int sign(int x) { return x > 0 ? 1 : x < 0 ? -1: 0; }

int main(void) {
    tap_plan(8);

    is(sign(pathcmp("/foo", "/foo")), 0);
    is(sign(pathcmp("/foo", "/foo/")), 0);
    is(sign(pathcmp("/foo//", "//foo/")), 0);
    is(sign(pathcmp("//foo//bar//", "/foo/bar")), 0);

    is(sign(pathcmp("/foo", "foo/")), -1);
    is(sign(pathcmp("/foo", "/fo/o")), 1);

    is(sign(pathcmp("/foo", "/bar")), 1);
    is(sign(pathcmp("/bar", "/foo")), -1);

    return tap_finish();
}
