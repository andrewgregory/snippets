#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>

#include "tap/tap.c"

#include "../mbscasecmp.c"

#define is(g, e) tap_is_int(g, e, "%s == %s", #g, #e)
#define ok(r) tap_ok(r, "%s", #r)

static int sign(int x) { return x > 0 ? 1 : x < 0 ? -1: 0; }

int main(void) {

    if(setlocale(LC_ALL, "") == NULL) {
        tap_bail("unable to set locale");
        return tap_finish();
    }

    tap_plan(19);
    is (mbscasecmp ("\303\266zg\303\274r", "\303\226ZG\303\234R"), 0); /* özgür */
    is (mbscasecmp ("\303\226ZG\303\234R", "\303\266zg\303\274r"), 0); /* özgür */

    /* This test shows how strings of different size can compare equal.  */
    is (mbscasecmp ("turkish", "TURK\304\260SH"), 0);
    is (mbscasecmp ("TURK\304\260SH", "turkish"), 0);

    is( mbscasecmp("Т", "т"), 0 );
    is( mbscasecmp("т", "т"), 0 );
    is( mbscasecmp("Т", "Т"), 0 );
    is( mbscasecmp("Y", "y"), 0 );
    is( mbscasecmp("Y", "Y"), 0 );
    is( mbscasecmp("y", "y"), 0 );

    ok( mbscasecmp("Т", "n") != 0 );
    ok( mbscasecmp("т", "n") != 0 );
    ok( mbscasecmp("Т", "N") != 0 );
    ok( mbscasecmp("Y", "n") != 0 );
    ok( mbscasecmp("Y", "N") != 0 );
    ok( mbscasecmp("y", "n") != 0 );

    is( sign(mbscasecmp("A", "a")), sign(strcasecmp("A", "a")) );
    is( sign(mbscasecmp("a", "A")), sign(strcasecmp("a", "A")) );
    is( sign(mbscasecmp("A", "Ac")), sign(strcasecmp("A", "Ac")) );

    return tap_finish();
}
