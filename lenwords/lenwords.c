#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DELIM ' '

void revwords_error(const char * error_str) {
    fprintf(stderr, "%s\n", error_str);
    exit(1);
}

char itoa_buf[24];
char word[4097];
size_t word_pos = 0;

void out_strlen() {
    sprintf(itoa_buf, "%lu", word_pos);
    write_(STDOUT_FILENO, itoa_buf, strlen(itoa_buf));
}

void out_char(char c) {
    write_(STDOUT_FILENO, &c, 1);
}

int main() {
    char buf[4096];
    size_t nread;

    do {
        nread = read_until(STDIN_FILENO, buf, sizeof(buf), DELIM);

        if (nread == -1) {
            revwords_error(strerror(errno));
        }

        for (int i = 0; i < nread; i++) {
            if (buf[i] == DELIM) {
                if (word_pos != 0) {
                    out_strlen();
                }
                word_pos = 0;
                out_char(DELIM);
            } else {
                word[word_pos++] = buf[i];
            }
        }

    } while (nread > 0);

    if (word_pos != 0) {
        out_strlen();
    }

    return 0;
}
