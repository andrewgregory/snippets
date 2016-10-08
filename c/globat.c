#define _GNU_SOURCE
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void _glob_freepattern(char **parts) {
    char **p;
    if(parts == NULL) { return; }
    for(p = parts; *p != NULL; p++) { free(*p); }
    free(parts);
}

char **_glob_split_pattern(const char *pattern) {
    size_t i, count = 1;
    char **parts;
    const char *c;

    if(pattern == NULL || pattern[0] == '\0') {
        return calloc(sizeof(char*), 1);
    }

    for(c = pattern; *c != '\0'; c++) {
        if(*c == '/') {
            count++;
            while(*c == '/') { c++; }
        }
    }

    if((parts = calloc(sizeof(char*), count + 1)) == NULL) {
        return NULL;
    }

    c = pattern;
    i = 0;
    if(*c == '/') {
        parts[i++] = strdup("/");
        while(*c == '/') { c++; }
    }
    while(1) {
        const char *sep = strchrnul(c, '/');
        parts[i++] = strndup(c, sep - c);

        if(sep[0] == '\0') { break; }

        while(sep[1] == '/') { sep++; }

        if(sep[1] == '\0') { parts[i++] = strdup("/"); break; }
        else { c = sep + 1; }
    }

    return parts;
}

char **_glob_split_pattern2(const char *pattern) {
    size_t i, count = 0;
    char **parts;
    const char *c;

    for(c = pattern + 1; *c != '\0'; c++) {
        int match = *c == '/';
        count += match;
        c += match;
    }

    if((parts = calloc(sizeof(char*), count + 1)) == NULL) {
        return NULL;
    }

    for(i = 0, c = pattern; *c != '\0'; i++) {
        const char *sep = strchrnul(c + 1, '/');
        if(sep[0] != '\0' && sep[1] == '\0') { sep += 1; }
        parts[i] = strndup(c, sep - c);
        if(sep[0] == '\0') { break; }
        c = sep + 1;
    }

    return parts;
}

int _glob_is_dir(int fd, const char *path) {
    struct stat buf;
    if(fstatat(fd, path, &buf, 0) != 0) {
        return -1;
    }
    return S_ISDIR(buf.st_mode);
}

int _glob_append(glob_t *pglob, char *path, int flags) {
    char **newmem;
    size_t newsize = pglob->gl_pathc + 2;

    if(flags & GLOB_DOOFFS) { newsize += pglob->gl_offs; }
    
    if(newsize < pglob->gl_pathc) { errno = ENOMEM; return -1; }
    if((newmem = realloc(pglob->gl_pathv, newsize * sizeof(char*))) == NULL) { return -1; }

    pglob->gl_pathv = newmem;
    pglob->gl_pathv[pglob->gl_pathc] = path;
    pglob->gl_pathv[pglob->gl_pathc + 1] = NULL;
    pglob->gl_pathc++;

    return 0;
}

int _globcmp(const void *p1, const void *p2) {
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

int _globat(int fd, char **pattern, int flags,
        int (*errfunc) (const char *epath, int eerrno),
        glob_t *pglob, const char *base) {
    const char *epath = (base && base[0]) ? base : ".";
    char path[PATH_MAX];
    DIR *dir;
    struct dirent *entry;
    int fnflags = FNM_PERIOD;

    if(!(flags & GLOB_APPEND)) {
        pglob->gl_pathc = 0;
        pglob->gl_pathv = NULL;
    }

    if(flags & GLOB_NOESCAPE) { fnflags |= FNM_NOESCAPE; }
#ifdef GLOB_PERIOD
    if(flags & GLOB_PERIOD) { fnflags &= ~FNM_PERIOD; }
#endif

#define MAYBE_ABORT(p, n) if( (errfunc && errfunc(p, n) != 0) \
        || flags & GLOB_ERR) \
    { return GLOB_ABORTED; }

    if((dir = fdopendir(fd)) == NULL) {
        int err = errno;
        close(fd);
        MAYBE_ABORT(epath, err)
        return 0;
    }

    while((errno = 0, entry = readdir(dir))) {
        struct stat sbuf;

        if(fnmatch(pattern[0], entry->d_name, fnflags) != 0) { continue; }

        if(base) {
            snprintf(path, PATH_MAX, "%s/%s", base, entry->d_name);
        } else {
            snprintf(path, PATH_MAX, "%s", entry->d_name);
        }

        if(fstatat(fd, entry->d_name, &sbuf, 0) != 0) {
            MAYBE_ABORT(path, errno);
            continue;
        }

        if(pattern[1] == NULL) {
            /* pattern is exhausted: match */
            if(S_ISDIR(sbuf.st_mode) && flags & GLOB_MARK) { strcat(path, "/"); }
            _glob_append(pglob, strdup(path), flags);
        } else if(!S_ISDIR(sbuf.st_mode)) {
            /* pattern is not exhausted, but entry is a file: no match */
        } else if(pattern[1][0] == '/') {
            /* pattern requires a directory and is exhausted: match */
            if(S_ISDIR(sbuf.st_mode) && flags & GLOB_MARK) { strcat(path, "/"); }
            _glob_append(pglob, strdup(path), flags);
        } else {
            /* pattern is not yet exhausted: check directory contents */
            int child = openat(fd, entry->d_name, O_DIRECTORY);
            if(child == -1) {
                MAYBE_ABORT(path, errno);
                continue;
            }

            int ret = _globat(child, pattern + 1,
                    flags | GLOB_APPEND | GLOB_NOSORT, errfunc, pglob, path);
            close(child);
            if(ret != 0) { closedir(dir); return ret; }
        }
    }
    closedir(dir);

    if(errno != 0 && GLOB_ERR) {
        return GLOB_ABORTED;
    }

    if(!(flags & GLOB_NOSORT)) {
        char **p = pglob->gl_pathv;
        if(flags & GLOB_DOOFFS) { p += pglob->gl_offs; }
        qsort(p, pglob->gl_pathc, sizeof(char*), _globcmp);
    }

    return 0;
}

int globat(int fd, const char *pattern, int flags,
        int (*errfunc) (const char *epath, int eerrno), glob_t *pglob) {
    char **parts, *base;
    int ret;

    if(pattern[0] == '/') {
        base = "";
        fd = open("/", O_DIRECTORY);
        while(pattern[0] == '/') { pattern++; }
    } else {
        fd = openat(fd, ".", O_DIRECTORY);
        base = NULL;
    }

    if(fd == -1) {
        return (flags & GLOB_ERR) ?  GLOB_ABORTED : GLOB_NOMATCH;
    }
    if((parts = _glob_split_pattern(pattern)) == NULL) {
        close(fd);
        return GLOB_NOSPACE;
    }

    ret = _globat(fd, parts, flags, errfunc, pglob, base);
    _glob_freepattern(parts);

    if(ret != 0 || pglob->gl_pathc > 0) {
        return ret;
    } else if(flags & GLOB_NOCHECK) {
        _glob_append(pglob, strdup(pattern), flags);
        return 0;
    } else {
        return GLOB_NOMATCH;
    }
}

int globat2(int fd, const char *pattern, int flags, glob_t *pglob) {
    int ret, cwd = open(".", O_DIRECTORY);

    fchdir(fd);
    ret = glob(pattern, flags, NULL, pglob);
    fchdir(cwd);
    close(cwd);

    return ret;
}

void globatfree(glob_t *g) {
	size_t i;
	for (i=0; i < g->gl_pathc; i++)
		free(g->gl_pathv[g->gl_offs + i]);
	free(g->gl_pathv);
	g->gl_pathc = 0;
	g->gl_pathv = NULL;
}

int globdir(const char *dir, const char *pattern, int flags,
        int (*errfunc) (const char *epath, int eerrno), glob_t *pglob) {
    int fd = open(dir, O_DIRECTORY);
    int ret = globat(fd, pattern, flags, errfunc, pglob);
    close(fd);
    return ret;
}

int main(int argc, char *argv[]) {
    size_t i;
    glob_t pglob = {0};
    int ret = globdir(argv[1], argv[2], 0, NULL, &pglob);
    printf("globdir(\"%s\", \"%s\") = %d\n", argv[1], argv[2], ret);
    printf("count: %zd\n", pglob.gl_pathc);
    for(i = 0; i < pglob.gl_pathc; i++) {
        puts(pglob.gl_pathv[i]);
    }
    globatfree(&pglob);
    return ret;
}
