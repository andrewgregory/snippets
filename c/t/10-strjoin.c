#include <stdlib.h>
#include "tap/tap.c"
#include "../strjoin.c"

int main() {
	tap_plan(7);

#define IS(g, e) { char *c = g; tap_is_str(c, e, #g); free(c); }
	IS( strjoin("/", "foo", "bar", "baz", NULL)  ,  "foo/bar/baz" );
	IS( strjoin("/", "foo", "bar", NULL)         ,  "foo/bar" );
	IS( strjoin("", "foo", "bar", NULL)          ,  "foobar" );
	IS( strjoin(NULL, "foo", "bar", NULL)        ,  "foobar" );
	IS( strjoin("foo", "", "", NULL)             ,  "foo" );
	IS( strjoin("", "", "", NULL)                ,  "" );
	IS( strjoin(NULL, NULL)                      ,  "" );
#undef IS

	return 0;
}
