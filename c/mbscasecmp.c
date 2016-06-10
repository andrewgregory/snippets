#include <string.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>

int mbscasecmp(const char *s1, const char *s2)
{
    size_t len1 = strlen(s1), len2 = strlen(s2);
    const char *p1 = s1, *p2 = s2;
    mbstate_t ps1, ps2;
    memset(&ps1, 0, sizeof(mbstate_t));
    memset(&ps2, 0, sizeof(mbstate_t));
    while(*p1 && *p2) {
        wchar_t c1, c2;
        size_t b1 = mbrtowc(&c1, p1, len1, &ps1);
        size_t b2 = mbrtowc(&c2, p2, len2, &ps2);
        if(b1 == (size_t) -2 || b1 == (size_t) -1
                || b2 == (size_t) -2 || b2 == (size_t) -1) {
            /* invalid multi-byte string, fall back to strcasecmp */
            return strcasecmp(p1, p2);
        }
        if(b1 == 0 || b2 == 0) {
            return c1 < c2 ? -1 : c1 > c2;
        }
        c1 = towlower(c1);
        c2 = towlower(c2);
        if(c1 != c2) {
            return c1 < c2 ? -1 : c1 > c2;
        }
        p1 += b1;
        p2 += b2;
        len1 -= b1;
        len2 -= b2;
    }
    return *p1 < *p2 ? -1 : *p1 > *p2;
}
