#include <bufio.h>

#include <sys/types.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

struct buf_t * buf_new(size_t capacity) {
    struct buf_t * buf = (struct buf_t *) malloc(sizeof(struct buf_t));
    if (buf == NULL) {
        return NULL;
    } else {
        buf->data = malloc(capacity);
        if (buf->data == NULL) {
            free(buf);
            return NULL;
        } else {
            buf->capacity = capacity;
            buf->size = 0;
            return buf;
        }
    }
}

void buf_free(struct buf_t * buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif

    free(buf->data);
    free(buf);
}

size_t buf_capacity(struct buf_t * buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif

    return buf->capacity;
}

size_t buf_size(struct buf_t * buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif

    return buf->size;
}

void buf_put(struct buf_t * buf, const char * src, size_t len) {
    #ifdef DEBUG
    if (buf == NULL || src == NULL || len > buf->capacity - buf->size) {
        abort();
    }
    #endif

    memcpy(buf->data + buf->size, src, len);
    buf->size += len;
}

ssize_t buf_fill(fd_t fd, struct buf_t * buf, size_t required) {
    #ifdef DEBUG
    if (buf == NULL || required > buf->capacity) {
        abort();
    }
    #endif

    ssize_t nread;

    do {
        nread = read(fd, buf->data + buf->size, buf->capacity - buf->size);

        if (nread == -1) {
            return -1;
        }

        buf->size += nread;
    } while (buf->size < required && nread > 0);

    return buf->size;
}

ssize_t buf_flush(fd_t fd, struct buf_t * buf, size_t required) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif

    size_t nall = 0;
    ssize_t nwritten;
    int error = 0;

    if (required > buf->size) {
        required = buf->size;
    }

    do {
        nwritten = write(fd, buf->data + nall, buf->size);

        if (nwritten == -1) {
            error = 1;
            break;
        }

        nall += nwritten;
        buf->size -= nwritten;
    } while (nall < required && nwritten > 0);

    memcpy(buf->data, buf->data + nall, buf->size);

    if (error) {
        return -1;
    } else {
        return nall;
    }
}

ssize_t buf_getline(fd_t fd, struct buf_t * buf, char * dest) {
    size_t prev_size = 0;
    ssize_t read_result;

    do {
        read_result = buf_fill(fd, buf, 1);

        if (read_result == -1) {
            return -1;
        }

        for (size_t i = prev_size; i < buf->size; i++) {
            if (buf->data[i] == '\n') {
                buf->size -= i + 1;
                memcpy(dest, buf->data, i + 1);
                memmove(buf->data, buf->data + i + 1, buf->size);
                return i + 1;
            }
        }
        prev_size = buf->size;
    } while (read_result > 0);

    return -1;
}

ssize_t buf_write(fd_t fd, struct buf_t * buf, char * src, size_t len) {
    size_t n_remain = len;

    while (n_remain > 0) {
        if (buf->capacity == buf->size) {
            if (buf_flush(fd, buf, 1) == -1) {
                return -1;
            }
        }
        
        size_t n_can_write = min(buf->capacity - buf->size, len);
        memcpy(buf->data + buf->size, src, n_can_write);
        n_remain -= n_can_write;
    }

    return len;
}
