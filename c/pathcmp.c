/* compare two paths for equivalence, ignores repeated and trailing '/' */
/* does NOT touch the filesystem */
int pathcmp(const char *p1, const char *p2)
{
    while(*p1 && *p1 == *p2) {
        /* skip repeated '/' */
        if(*p1 == '/') {
            while(*(++p1) == '/');
            while(*(++p2) == '/');
        } else {
            p1++;
            p2++;
        }
    }

    /* skip trailing '/' */
    if(*p1 == '\0') {
        while(*p2 == '/') { p2++; }
    } else if(*p2 == '\0') {
        while(*p1 == '/') { p1++; }
    }

    return *p1 - *p2;
}
