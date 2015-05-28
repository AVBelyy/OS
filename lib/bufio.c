#include <bufio.h>

#include <sys/types.h>

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

ssize_t buf_fill(fd_t fd, struct buf_t * buf, size_t required) {
    #ifdef DEBUG
    if (buf == NULL || required > buf->capacity) {
        abort();
    }
    #endif

    ssize_t nread;

    buf->size = 0;

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
        return buf->size;
    }
}
