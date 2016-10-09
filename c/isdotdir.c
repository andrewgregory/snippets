/* simple inline-able string test for "." or ".." */
inline int isdotdir(const char *c) {
    return (c[0] == '.' && (c[1] == '\0' || (c[1] == '.' && c[2] == '\0')));
}

/* macro equivalent */
#define isdotdir2(s) \
    (s[0] == '.' && (s[1] == '\0' || (s[1] == '.' && s[2] == '\0')))
