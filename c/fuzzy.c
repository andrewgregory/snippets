#include <string.h>
#include <ctype.h>

char *strcasechr(char *haystack, int c) {
    char u = toupper(c), l = tolower(c);
    while(*haystack && *haystack != u && *haystack != l) { haystack++; }
    return *haystack == '\0' ? NULL : haystack;
}

char *strfuzzy(char *haystack, const char *needle) {
    if(!haystack || !needle) { return NULL; }
    haystack = strchr(haystack, *(needle++));
    while(haystack && *needle) {
        haystack = strchr(haystack + 1, *(needle++));
    }
    return haystack;
}

char *strcasefuzzy(char *haystack, const char *needle) {
    if(!haystack || !needle) { return NULL; }
    haystack = strcasechr(haystack, *(needle++));
    while(haystack && *needle) {
        haystack = strcasechr(haystack + 1, *(needle++));
    }
    return haystack;
}
