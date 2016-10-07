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

static int _glob_isdotdir(const char *c) {
    return (c[0] == '.' && (c[1] == '\0' || (c[1] == '.' && c[2] == '\0')));
}

void _glob_freepattern(char **parts) {
    char **p;
    if(parts == NULL) { return; }
    for(p = parts; *p != NULL; p++) { free(*p); }
    free(parts);
}

void globatfree(glob_t *g) {
	size_t i;
	for (i=0; i < g->gl_pathc; i++)
		free(g->gl_pathv[g->gl_offs + i]);
	free(g->gl_pathv);
	g->gl_pathc = 0;
	g->gl_pathv = NULL;
}

char **_glob_split_pattern(const char *pattern) {
    size_t i, count = 0;
    char **parts;
    const char *c;
    for(c = pattern + 1; *c != '\0'; c++) {
        count += *c == '/';
        c += *c == '/';
    }
    parts = calloc(sizeof(char*), count + 5);
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

int _globat(int fd, char **pattern, int flags, glob_t *pglob, const char *base) {
    char path[PATH_MAX];
    int dirfd;
    DIR *dir;
    struct dirent *entry;
    int fnflags = FNM_PERIOD;

    if((dirfd = openat(fd, ".", O_DIRECTORY)) == -1) { return GLOB_ABORTED; }
    if((dir = fdopendir(dirfd)) == NULL) { close(dirfd); return GLOB_ABORTED; }

    if(!(flags & GLOB_APPEND)) {
        pglob->gl_pathc = 0;
        pglob->gl_pathv = NULL;
    }

    if(flags & GLOB_PERIOD) { fnflags &= ~FNM_PERIOD; }

    while((errno = 0, entry = readdir(dir))) {
        struct stat sbuf;

        if(_glob_isdotdir(entry->d_name)) { continue; }
        if(fnmatch(pattern[0], entry->d_name, fnflags) != 0) { continue; }

        if(base && base[0]) {
            snprintf(path, PATH_MAX, "%s/%s", base, entry->d_name);
        } else {
            snprintf(path, PATH_MAX, "%s", entry->d_name);
        }

        if(fstatat(fd, entry->d_name, &sbuf, 0) != 0) {
            /* errfunc(path, errno) */
            if(flags & GLOB_ERR) { return GLOB_ABORTED; }
            else { continue; }
        }

        if(pattern[1] == NULL) {
            /* pattern is exhausted: match */
            if(S_ISDIR(sbuf.st_mode) && flags & GLOB_MARK) { strcat(path, "/"); }
            _glob_append(pglob, strdup(path), flags);
        } else if(!S_ISDIR(sbuf.st_mode)) {
            /* pattern is not exhausted, but entry is a file: no match */
        } else if(pattern[1] == "/") {
            /* pattern requires a directory and is exhausted: match */
            if(S_ISDIR(sbuf.st_mode) && flags & GLOB_MARK) { strcat(path, "/"); }
            _glob_append(pglob, strdup(path), flags);
        } else {
            /* pattern is not yet exhausted: check directory contents */
            int child = openat(fd, entry->d_name, O_DIRECTORY);
            int ret = _globat(child, pattern + 1, flags | GLOB_APPEND | GLOB_NOSORT, pglob, path);
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

int globat(int fd, const char *pattern, int flags, glob_t *pglob) {
    if(fd == AT_FDCWD || pattern[0] == '/')
        { return glob(pattern, flags, NULL, pglob); }
    char **parts = _glob_split_pattern(pattern);
    int ret = _globat(fd, parts, flags, pglob, "");
    _glob_freepattern(parts);
    return ret;
}

int globat2(int fd, const char *pattern, int flags, glob_t *pglob) {
    int ret, cwd = open(".", O_DIRECTORY);

    fchdir(fd);
    ret = glob(pattern, flags, NULL, pglob);
    fchdir(cwd);
    close(cwd);

    return ret;
}

int globdir(const char *dir, const char *pattern, int flags, glob_t *pglob) {
    int fd = open(dir, O_DIRECTORY);
    int ret = globat(fd, pattern, flags, pglob);
    close(fd);
    return ret;
}

#include <stdio.h>
int main(int argc, char *argv[]) {
    size_t i;
    glob_t pglob = {0};
    int ret = globdir(argv[1], argv[2], 0, &pglob);
    printf("globdir(\"%s\", \"%s\") = %d\n", argv[1], argv[2], ret);
    printf("count: %zd\n", pglob.gl_pathc);
    for(i = 0; i < pglob.gl_pathc; i++) {
        puts(pglob.gl_pathv[i]);
    }
    globatfree(&pglob);
    return ret;
}

