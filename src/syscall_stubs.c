#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <uart.h>

/** @brief Built-in file descriptors */
//@{
#define FD_STDIN    0
#define FD_STDOUT   1
//@}

/** @brief _sbrk allocates more area in the heap which can be used
 *  by the user caller */
void *_sbrk(int incr) {
    (void)incr;
    return NULL;
}

/** @brief _write allows the user to write things to STDOUT */
int _write(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return -1;
}

/** @brief _read allows the user to read from STDIN */
int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return -1;
}

/** @brief _exit exits from the current user program by printing the
 *  status, then going into sleep mode. */
void _exit(int status) {
    (void)status;
    while (1);
}

int _close(int file) {
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    if (file == FD_STDIN || file == FD_STDOUT) return 1;
    else return 0;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}
