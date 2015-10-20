#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* shell style word splitting on whitespace with quote handling */
/* both ' and " are recognized as quotes */
/* returns NULL on errors */
/* if <str> is NULL or contains unbalanced quotes errno is set to EINVAL */
/*
  wordsplit("foo bar") => { "foo", "bar", NULL }
  wordsplit(" foo  'bar baz'  quux ") => { "foo", "bar baz", "quux", NULL }

  wordsplit(" foo\\'bar ") => { "foo'bar", NULL }
  wordsplit(" '\\'foo\\' \\\"bar\"' ") => { "'foo' \\\"bar\"", NULL }
  wordsplit(" fo'ob'ar ") => { "foobar", NULL }

  wordsplit(" '' ") => { "", NULL }
  wordsplit("  ") => { NULL }

  wordsplit(NULL) => NULL
  wordsplit("'") => NULL
*/

void wordsplit_free(char **ws) {
    if(ws) {
        char **c;
        for(c = ws; *c; c++) {
            free(*c);
        }
        free(ws);
    }
}

char **wordsplit(char *str) {
    char *c = str, *end;
    char **out = NULL, **outsave;
    size_t count = 0;

    if(str == NULL) {
        errno = EINVAL;
        return NULL;
    }

    while(isspace(*c)) { c++; }
    while(*c) {
        size_t wordlen = 0;

        /* extend our array */
        outsave = out;
        if((out = realloc(out, (count + 1) * sizeof(char*))) == NULL) {
            out = outsave;
            goto error;
        }

        /* calculate word length and check for errors */
        for(end = c; *end && !isspace(*end); end++) {
            if(*end == '\'' || *end == '"') {
                char quote = *end;
                while(*(++end) && *end != quote) {
                    if(*end == '\\' && *(end + 1) == quote) {
                        end++;
                    }
                    wordlen++;
                }
                if(*end != quote) {
                    errno = EINVAL;
                    goto error;
                }
            } else if(*end == '\\' && (end[1] == '\'' || end[1] == '"')) {
                end++;
                wordlen++;
            } else {
                wordlen++;
            }
        }

        if(wordlen == (size_t) (end - c)) {
            /* no internal quotes or escapes, copy it the easy way */
            if((out[count++] = strndup(c, wordlen)) == NULL) { goto error; }
        } else {
            /* manually copy to remove quotes and escapes */
            char *dest = out[count++] = malloc(wordlen + 1);
            if(dest == NULL) { goto error; }
            while(c < end) {
                if(*c == '\'' || *c == '"') {
                    char quote = *c;
                    int escaped = 0;
                    /* we know there must be a matching end quote,
                     * no need to check for '\0' */
                    for(c++; escaped || *c != quote; c++) {
                        if(*c == '\\' && *(c + 1) == quote) {
                            escaped = 1;
                        } else {
                            escaped = 0;
                            *(dest++) = *c;
                        }
                    }
                    c++;
                } else if(*c == '\\' && (c[1] == '\'' || c[1] == '"')) {
                    c++; /* skip the '\\' */
                    *(dest++) = *(c++); /* copy the quote directly */
                } else {
                    *(dest++) = *(c++);
                }
            }
            *dest = '\0';
        }

        if(*end == '\0') {
            break;
        } else {
            for(c = end + 1; isspace(*c); c++);
        }
    }

    outsave = out;
    if((out = realloc(out, (count + 1) * sizeof(char*))) == NULL) {
        out = outsave;
        goto error;
    }

    out[count++] = NULL;

    return out;

error:
    /* can't use wordsplit_free here because NULL has not been appended */
    while(count) {
        free(out[--count]);
    }
    free(out);
    return NULL;
}
