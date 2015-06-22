#include <helpers.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void cat_error(const char * error_str) {
    fprintf(stderr, "%s\n", error_str);
    exit(1);
}

int main() {
    char buf[4096];
    size_t nread;

    do {
        nread = read_(STDIN_FILENO, buf, sizeof(buf));

        if (nread == -1) {
            cat_error(strerror(errno));
        }

        size_t nwritten = write_(STDOUT_FILENO, buf, nread);

        if (nwritten < nread) {
            cat_error("Unexpected EOF in output");
        }
    } while (nread > 0);

    return 0;
}
