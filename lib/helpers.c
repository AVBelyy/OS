#include <helpers.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h> // TODO : for debug purposes only
#include <signal.h>

ssize_t read_until(int fd, void * buf, size_t count, char delimeter) {
    size_t nall = 0;
    ssize_t nread;
    int delim_found = 0;

    if (count == 0) {
        return read(fd, buf, 0);
    }

    do {
        nread = read(fd, buf + nall, count);

        if (nread == -1) {
            return -1;
        }

        for (int i = 0; i < nread; i++) {
            if (((char*) buf)[nall + i] == delimeter) {
                delim_found = 1;
                break;
            }
        }

        nall += nread;
        count -= nread;
    } while (count > 0 && nread > 0 && !delim_found);

    return nall;
}

ssize_t read_(int fd, void * buf, size_t count) {
    size_t nall = 0;
    ssize_t nread;

    if (count == 0) {
        return read(fd, buf, 0);
    }

    do {
        nread = read(fd, buf + nall, count);

        if (nread == -1) {
            return -1;
        }

        nall += nread;
        count -= nread;
    } while (count > 0 && nread > 0);

    return nall;
}

ssize_t write_(int fd, const void * buf, size_t count) {
    size_t nall = 0;
    ssize_t nwritten;

    if (count == 0) {
        return write(fd, buf, 0);
    }

    do {
        nwritten = write(fd, buf + nall, count);

        if (nwritten == -1) {
            return -1;
        }

        nall += nwritten;
        count -= nwritten;
    } while (count > 0 && nwritten > 0);

    return nall;
}

int spawn(const char * file, char * const argv []) {
    pid_t pid = fork();
    if (pid == -1) {
        return SPAWN_UNEXPECTED_RESULT;
    } else if (pid == 0) {
        execvp(file, argv);
        return SPAWN_UNEXPECTED_RESULT;
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return SPAWN_UNEXPECTED_RESULT;
        }
    }
}

struct execargs_t * new_execargs(const char * file, char ** argv) {
    struct execargs_t * args = malloc(sizeof(struct execargs_t));

    args->file = file;
    args->argv = argv;

    return args;
}

int exec(struct execargs_t * args) {
    return spawn(args->file, args->argv);
}

// Signal handler for child process of runpiped(...)
void exit_with_error() {
    exit(1);
}

int runpiped(struct execargs_t ** programs, size_t n) {
    // Create all necessary pipes
    int pipefd[2 * (n - 1)];
    for (size_t i = 0; i < n - 1; i++) {
        if (pipe(pipefd + 2 * i) == -1) {
            return -1;
        }
    }

    // Mask SIGCHLD, SIGINT & make them queued
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    struct sigaction action;
    action.sa_handler = SIG_IGN; // TODO : try sth else
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGCHLD, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // Start child processes
    int pids[n];
    for (size_t i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            // TODO : kill previously created children
            return -1;
        }
        if (pid == 0) {
            // Connect STDIN and STDOUT to pipes
            if (i != 0) {
                dup2(pipefd[2 * i - 2], STDIN_FILENO);
            }
            if (i != n - 1) {
                dup2(pipefd[2 * i + 1], STDOUT_FILENO);
            }

            // Close irrelevant pipes
            for (size_t j = 0; j < 2 * (n - 1); j++) {
                close(pipefd[i]);
            }

            // Unmask SIGCHLD and SIGINT
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            
            // Exit with error on SIGPIPE
            struct sigaction action;
            action.sa_handler = exit_with_error;
            sigemptyset(&action.sa_mask);
            action.sa_flags = 0;
            sigaction(SIGPIPE, &action, NULL);

            // Finally, execute target program
            execvp(programs[i]->file, programs[i]->argv);
            return -1;
        } else {
            pids[i] = pid;
        }
    }

    // Wait for queued signals to arrive
    siginfo_t info;
    for (;;) {
        if (sigwaitinfo(&mask, &info) < 0) {
            continue;
        }
        if (info.si_signo == SIGCHLD) {
            printf("a child terminated, pid = %d\n", info.si_pid);
        } else /* if (info.si_signo == SIGINT) */ {
            printf("sigint arrived\n");
        }
    }
    
    // Close all pipes
    for (size_t i = 0; i < 2 * (n - 1); i++) {
        close(pipefd[i]);
    }
    return 0;
}
