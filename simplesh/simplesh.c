#include <helpers.h>
#include <bufio.h>

const size_t BUF_SIZE = 4096;
const size_t ARG_SIZE = 4096;
const size_t ARGS_COUNT = 2048;
size_t n;
char line[BUF_SIZE + 1];
char prompt[] = "\033[01;32m$\033[00m";

struct execargs_t * parse_piped_cmd(size_t start, size_t end) {
    char arg_buf[ARG_SIZE];
    size_t buf_pos = 0;

    // TODO : Because I'm too lazy to precount args count
    char ** args = malloc(ARGS_COUNT * sizeof(char *));
    size_t arg_count = 0;

    for (size_t i = start; i < end; i++) {
        if (line[i] == ' ') {
            if (buf_pos > 0) {
                args[arg_count] = malloc(buf_pos);
                memcpy(args[arg_count], arg_buf, buf_pos);
                arg_count++;
                buf_pos = 0;
            }
        } else {
            arg_buf[buf_pos++] = line[i];
        }
    }
    if (buf_pos > 0) {
        args[arg_count] = malloc(buf_pos);
        memcpy(args[arg_count], arg_buf, buf_pos);
        arg_count++;
    }
    args[arg_count] = NULL;

    return new_execargs(args[0], args);
}

struct execargs_t ** parse_line(size_t start, size_t end) { 
    // Count length and number of '|'s in a line
    n = 1;
    for (size_t i = start; i < end; i++) {
        if (line[i] == '|') {
            n++;
        }
    }

    // Create array of execargs
    struct execargs_t ** programs = malloc(n * sizeof(struct execargs_t *));

    // Parse piped commands
    size_t left = start, prg_count = 0;
    for (size_t i = start; i < end; i++) {
        if (line[i] == '|') {
            programs[prg_count++] = parse_piped_cmd(left, i);
            left = i + 1;
        }
    }
    programs[prg_count] = parse_piped_cmd(left, end);

    return programs;
}

int main() {
    struct buf_t * io_buf = buf_new(BUF_SIZE);
    ssize_t nread;

    for (;;) {
        write_(STDOUT_FILENO, prompt, sizeof(prompt)); 
        nread = buf_getline(STDIN_FILENO, io_buf, line);

        if (nread <= 0) {
            break;
        }
        
        struct execargs_t ** programs = parse_line(0, nread);
        runpiped(programs, n);
    }

    // Cleanup.
    buf_free(io_buf);
    return 0;
}
