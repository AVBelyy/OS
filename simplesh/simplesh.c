#define _POSIX_SOURCE

#include <helpers.h>
#include <bufio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define BUF_SIZE 4096
#define ARG_SIZE 4096

size_t n;
char line[BUF_SIZE + 1];
char prompt_fmt[] = "%d \033[01;32m$ \033[00m";

struct execargs_t * parse_piped_cmd(size_t start, size_t end) {
    // Precalc args count
    size_t arg_count = 0;
    size_t buf_pos = 0;
    for (size_t i = start; i < end; i++) {
        if (isspace(line[i])) {
            if (buf_pos > 0) {
                arg_count++;
                buf_pos = 0;
            }
        } else {
            buf_pos++;
        }
    }
    if (buf_pos > 0) {
        arg_count++;
    }

    char arg_buf[ARG_SIZE];
    char ** args = malloc((arg_count + 1) * sizeof(char *));
    size_t arg_num = 0;
    buf_pos = 0;

    for (size_t i = start; i < end; i++) {
        if (isspace(line[i])) {
            if (buf_pos > 0) {
                args[arg_num] = malloc(buf_pos + 1);
                memcpy(args[arg_num], arg_buf, buf_pos);
                args[arg_num][buf_pos] = 0;
                arg_num++;
                buf_pos = 0;
            }
        } else {
            arg_buf[buf_pos++] = line[i];
        }
    }
    if (buf_pos > 0) {
        args[arg_num] = malloc(buf_pos + 1);
        memcpy(args[arg_num], arg_buf, buf_pos);
        args[arg_num][buf_pos] = 0;
        arg_num++;
    }
    args[arg_num] = NULL;

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
    size_t left = start, prg_num = 0;
    for (size_t i = start; i < end; i++) {
        if (line[i] == '|') {
            programs[prg_num++] = parse_piped_cmd(left, i);
            left = i + 1;
        }
    }
    programs[prg_num] = parse_piped_cmd(left, end);

    return programs;
}

int sigint_flag = 0;
void sigint_handler() {
    sigint_flag = 1;
}

void set_signal_handler(int signo, void (*handler)(int)) {
    struct sigaction action;
    action.sa_handler = handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(signo, &action, NULL);
}

int main() {
    struct buf_t * io_buf = buf_new(BUF_SIZE);
    ssize_t nread;
    int retcode = 0;

    set_signal_handler(SIGINT, sigint_handler);

    for (;;) {
        char prompt_buf[256];
        int prompt_len = sprintf(prompt_buf, prompt_fmt, retcode);
        write_(STDOUT_FILENO, prompt_buf, prompt_len);
        nread = buf_getline(STDIN_FILENO, io_buf, line);

        if (nread <= 0) {
            if (errno == EINTR && sigint_flag == 1) {
                // Continue as if nothing happened
                int newline = '\n';
                write_(STDOUT_FILENO, &newline, 1);
                errno = 0;
                sigint_flag = 0;
                continue;
            } else {
                break;
            }
        }
        
        struct execargs_t ** programs = parse_line(0, nread);
        retcode = runpiped(programs, n);
    }

    // Cleanup.
    buf_free(io_buf);
    return 0;
}
