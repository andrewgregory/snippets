#include <stdlib.h>
#include "tap/tap.c"
#include "../asprintf.c"

int main() {
	tap_plan(2);

#define IS(g, e) { char *c = g; tap_is_str(c, e, "%s", #g); free(c); }
	IS( asprintf("%s/%s/%s", "foo", "bar", "baz") ,  "foo/bar/baz" );
	IS( asprintf("%d - %s", 3, "foo")             ,  "3 - foo" );
#undef IS

	return tap_finish();
}
