#include <helpers.h>
#include <bufio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char line[4097];

char ** spawn_argv;

void out_str(int fd, const char * str) {
    write(fd, str, strlen(str));
}

void out_char(int fd, char c) {
    write(fd, &c, 1);
}

void exec_on_line() {
    if (spawn(spawn_argv[0], spawn_argv) == 0) {
        out_str(STDOUT_FILENO, line);
        out_char(STDOUT_FILENO, '\n');
    }
}

int main(int argc, char ** argv) {
    struct buf_t * buf = buf_new(4096);
    ssize_t nread;

    // Prepare spawn_argv.
    if (argc < 2) {
        out_str(STDERR_FILENO, "incorrect args\n");
        return 1;
    } else {
        spawn_argv = malloc(sizeof(char *) * (argc + 1));
        for (int i = 0; i < argc - 1; i++) {
            spawn_argv[i] = argv[i + 1];
        }
        spawn_argv[argc - 1] = line;
        spawn_argv[argc] = NULL;
    }

    do {
        nread = buf_getline(STDIN_FILENO, buf, line);

        line[nread] = 0;
        printf("'%s' %d\n", buf->data, nread);
        exec_on_line();
    } while (nread > 0);

    // Cleanup.
    free(buf);
    free(spawn_argv);

    return 0;
}
