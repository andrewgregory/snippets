#include <string.h>

/* locale-insensitive isspace(3) */
int iscspace(int c) {
    return strchr(" \f\n\r\t\v", c) ? 1 : 0;
}

/* trim leading and trailing whitespace (locale-insensitive) from a string */
/* returns the new string length */
size_t strtrim(char *str) {
    register char *start, *end;

    if(!str || !*str) { return 0; }

    for(start = str; iscspace((int) *start); start++);
    for(end = start + strlen(start) - 1; iscspace((int) *end); end--);

    *(++end) = '\0';
    memmove(str, start, end - start + 1);

    return end - start;
}
