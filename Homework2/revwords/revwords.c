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

char word[4097];
size_t word_pos = 0;

void out_reverse() {
    for (int j = 0; j < word_pos / 2; j++) {
        char t = word[j];
        word[j] = word[word_pos - j - 1];
        word[word_pos - j - 1] = t;
    }
    write_(STDOUT_FILENO, word, word_pos);
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
                    out_reverse();
                }
                word_pos = 0;
                out_char(DELIM);
            } else {
                word[word_pos++] = buf[i];
            }
        }

    } while (nread > 0);

    if (word_pos != 0) {
        out_reverse();
    }

    return 0;
}
