#include <bufio.h>

int main() {
    int retcode = 0;
    struct buf_t * buf = buf_new(4096);
    ssize_t nread, nwritten;

    do {
        nread = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
        nwritten = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
        if (nread == -1 || nwritten == -1) {
            retcode = 1;
            break;
        }
    } while (nread > 0);

    buf_free(buf);
    return retcode;
}
