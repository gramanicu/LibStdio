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

struct _so_file {
    unsigned char _buf[BUFSIZE];

    int _fd;
    int _flags;

    int _read_pos;

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

    stream->_fd = open(pathname, stream->_flags, 0600);

    if (stream->_fd == -1) {
        free(stream);
        return NULL;
    }

    /* Move the read pointer */
    stream->_read_pos = 0;

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

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream) { return 0; }

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence) {}
FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream) {}

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    size_t bytes_read = 0;
    size_t bytes_current = 0;
    size_t bytes_total = size * nmemb;

    if (stream->_eof) { return 0; }

    while (bytes_current < bytes_total) {
        /* Make sure we have data in the buffer */
        if (stream->_read_pos == 0) {
            /* Buffer empty */
            size_t read_s;
            read_s = read(stream->_fd, stream->_buf, BUFSIZE);

            if (read_s < 0) {
                /* Read error */
                stream->_err = 1;
                return SO_EOF;
            }

            if (read_s == 0) { stream->_eof = 1; }
        }

        /* Compute the remaining amount of data that will be read */
        int size_to_read = bytes_total - bytes_current;
        int udata_size = BUFSIZE - stream->_read_pos;
        size_to_read = min(size_to_read, udata_size);

        /* Move the data from the buffer */
        memcpy(ptr + bytes_current, stream->_buf + stream->_read_pos,
               size_to_read);

        stream->_read_pos += size_to_read;
        if (stream->_read_pos == BUFSIZE) { stream->_read_pos = 0; }

        bytes_current += size_to_read;
    }

    return bytes_current / size;
}

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream) {
    size_t read_c;
    unsigned char ret_val;

    if (stream->_read_pos == 0) {
        /* Buffer empty */
        read_c = read(stream->_fd, stream->_buf, BUFSIZE);

        if (read < 0) {
            /* Read error */
            return SO_EOF;
        }

        ret_val = stream->_buf[stream->_read_pos];
        stream->_read_pos += 1;
    } else {
        ret_val = stream->_buf[stream->_read_pos];
        stream->_read_pos += 1;

        if (stream->_read_pos == BUFSIZE) { stream->_read_pos = 0; }
    }

    return ret_val;
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream) {}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream) {}
FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream) {}

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type) {}
FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream) {}