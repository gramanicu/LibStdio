/**
 * @file so_stdio.c
 * @author Grama Nicolae (gramanicu@gmail.com)
 * @brief The implementation for the so_stdio library
 * @copyright Copyright (c) 2021
 */

#include "so_stdio.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "error_handling.h"

#define BUFSIZE 4096
#define OP_READ 1
#define OP_WRITE -1

struct _so_file {
    unsigned char _buf[BUFSIZE];

    int _fd;
    int _flags;

    int _position;
    int _last_op;
    int _last_read;
    int _total_bytes;

    int _eof;
    int _err;
};

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream) { return stream->_fd; }

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode) {
    SO_FILE *stream = calloc(1, sizeof(SO_FILE));

    if (stream == NULL) { return NULL; }

    if (strcmp(mode, "r") == 0) {
        stream->_flags = O_RDONLY;
    } else if (strcmp(mode, "r+") == 0) {
        stream->_flags = O_RDWR;
    } else if (strcmp(mode, "w") == 0) {
        stream->_flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "w+") == 0) {
        stream->_flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0) {
        stream->_flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "a+") == 0) {
        stream->_flags = O_RDWR | O_CREAT | O_APPEND;
    } else {
        free(stream);
        return NULL;
    }

    stream->_fd = open(pathname, stream->_flags, 0644);

    if (stream->_fd == -1) {
        free(stream);
        return NULL;
    }

    /* Move the read pointer */
    stream->_position = 0;

    return stream;
}

FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream) {
    int rc;

    rc = so_fflush(stream);
    if (rc < 0) { return SO_EOF; }

    rc = close(stream->_fd);
    if (rc < 0) { return SO_EOF; }

    free(stream);

    return 0;
}

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream) {
    int to_write;
    ssize_t last_write = 0;
    ssize_t total_write = 0;

    if (stream->_last_op != OP_WRITE) {
        memset(stream->_buf, 0, BUFSIZE);
        stream->_position = 0;
        stream->_last_op = 0;
        return 0;
    }

    to_write = stream->_position;
    if (stream->_last_op == OP_WRITE) {
        if (to_write < 0) { return SO_EOF; }

        while (to_write > 0) {
            last_write =
                write(stream->_fd, stream->_buf + total_write, to_write);

            if (last_write < 0) {
                stream->_err = 1;
                return SO_EOF;
            }

            to_write -= last_write;
            total_write += last_write;
        }

        memset(stream->_buf, 0, BUFSIZE);
        stream->_position = 0;
        stream->_last_op = 0;
    }

    return 0;
}

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence) {
    int rc;

    rc = so_fflush(stream);
    if (rc < 0) {
        stream->_err = 1;
        return SO_EOF;
    }

    if (whence == SEEK_END || whence == SEEK_CUR || whence == SEEK_SET) {
        rc = lseek(stream->_fd, offset, whence);

        if (rc < 0) {
            stream->_err = 1;
            return SO_EOF;
        }

        stream->_total_bytes = rc;
    }
    return 0;
}

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream) {
    return stream->_position + stream->_total_bytes;
}

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    size_t bytesRead = nmemb * size;
    size_t i;
    int val;
    uchar *p = ptr;

    if (stream == NULL) { return SO_EOF; }

    for (i = 0; i < bytesRead; ++i) {
        val = so_fgetc(stream);

        if (val < 0) { return SO_EOF; }

        *p = val;
        p++;
    }

    return nmemb;
}

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    size_t bytesWritten = size * nmemb;
    size_t i;
    int val, rc;
    uchar *p = ptr;

    if (stream == NULL) { return SO_EOF; }

    for (i = 0; i < bytesWritten; ++i) {
        val = *p;
        rc = so_fputc(val, stream);

        if (rc < 0) { return SO_EOF; }

        p++;
    }

    return nmemb;
}

int refillBuffer(SO_FILE *stream) {
    ssize_t read_c;
    /* Buffer empty */
    read_c = read(stream->_fd, stream->_buf, BUFSIZE);

    if (read < 0) {
        /* Read error */
        stream->_err = 1;
        stream->_last_op = OP_READ;
        return SO_EOF;
    }

    if (read == 0) {
        /* EOF */
        stream->_eof = 1;
        return SO_EOF;
    }

    stream->_last_read = read_c;
    stream->_total_bytes = read_c;
    return 0;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream) {
    unsigned char val;
    int read_again = 0;

    /* If we "exhausted" the buffer */
    if (stream->_position >= stream->_last_read) {
        stream->_position = 0;
        stream->_last_read = 0;
        read_again = 1;
    }

    if (read_again) {
        /* Buffer empty */
        if (refillBuffer(stream) != 0) { return SO_EOF; }
    }

    val = stream->_buf[stream->_position];
    stream->_last_op = OP_READ;
    stream->_position += 1;

    return val;
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream) {
    int rc;
    stream->_last_op = OP_WRITE;

    if (stream->_position == BUFSIZE) {
        stream->_total_bytes = stream->_position;
        rc = so_fflush(stream);
        if (rc == SO_EOF) {
            stream->_err = 1;
            return SO_EOF;
        }
    }

    stream->_buf[stream->_position] = c;
    stream->_position += 1;

    return c;
}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream) { return stream->_eof; }
FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream) { return stream->_err; }

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type) {}
FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream) {}