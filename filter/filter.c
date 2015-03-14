#include <helpers.h>
#include <stdlib.h>
#include <string.h>

// Assuming that maximum length of a line is 4096.

char line[4097];
size_t line_pos = 0;

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
    char buf[4096];
    size_t nread;

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
        nread = read_until(STDIN_FILENO, buf, sizeof(buf), '\n');

        for (int i = 0; i < nread; i++) {
            if (buf[i] == '\n') {
                if (line_pos != 0) {
                    line[line_pos] = 0;
                    exec_on_line();
                }
                line_pos = 0;
            } else {
                line[line_pos++] = buf[i];
            }
        }

    } while (nread > 0);

    if (line_pos != 0) {
        line[line_pos] = 0;
        exec_on_line();
    }

    // Cleanup.
    free(spawn_argv);

    return 0;
}
