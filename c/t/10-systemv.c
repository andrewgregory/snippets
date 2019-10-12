#include "tap/tap.c"

#include "../systemv.c"

int main(void) {
    char *cmd[] = { "/bin/sh", "-c", "exit 0", NULL };

    tap_plan(9);

    tap_is_int( systemv(cmd[0], cmd), 0, "exit 0" );
    errno = 0;
    tap_is_int( systemv("non-existent", cmd), -1, "non-existent file - return" );
    tap_is_int( errno, ENOENT, "non-existent file - errno" );

    tap_is_int( systemvp("sh", cmd), 0, "exit 0" );
    errno = 0;
    tap_is_int( systemvp("non-existent", cmd), -1, "non-existent file - return" );
    tap_is_int( errno, ENOENT, "non-existent file - errno" );

    tap_is_int( systeml("/bin/sh", "/bin/sh", "-c", "exit 0", NULL), 0, "exit 0" );
    errno = 0;
    tap_is_int( systeml("non-existent", "foo", NULL), -1, "non-existent file - return" );
    tap_is_int( errno, ENOENT, "non-existent file - errno" );

    return tap_finish();
}
