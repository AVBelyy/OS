#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

#define SPAWN_UNEXPECTED_RESULT -1

struct execargs_t {
    const char * file;
    char ** argv;
};

ssize_t read_(int fd, void * buf, size_t count);
ssize_t write_(int fd, const void * buf, size_t count);
ssize_t read_until(int fd, void * buf, size_t count, char delimeter);
int spawn(const char * file, char * const argv []);
struct execargs_t * new_execargs(const char * file, char ** argv);
int exec(struct execargs_t * args);
int runpiped(struct execargs_t ** programs, size_t n);

#endif
