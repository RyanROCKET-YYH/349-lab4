#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <uart.h>

/** @brief Built-in file descriptors */
//@{
#define FD_STDIN    0
#define FD_STDOUT   1
//@}

extern char __heap_low;  // Start of the heap
extern char __heap_top;  // End of the heap (exclusive)

/** @brief _sbrk allocates more area in the heap which can be used
 *  by the user caller */
void *_sbrk(int incr) {
    static char* current_heap_end; // Current "break" (end of the heap)
    char* previous_heap_end;
    if (current_heap_end == 0) {
        current_heap_end = &__heap_low;
    }
    previous_heap_end = current_heap_end;

    // Check if the new heap end exceeds the heap top
    if (current_heap_end + incr > &__heap_top) {
        // We don't have enough memory for this allocation
        return (void*)-1;
    }

    current_heap_end += incr; // Increase the heap
    return previous_heap_end; // Return the previous break
}

/** @brief _write allows the user to write things to STDOUT */
int _write(int file, char *ptr, int len) {
    // call uart write
    return uart_write(file, ptr, len);
}

/** @brief _read allows the user to read from STDIN */
int _read(int file, char *ptr, int len) {
    // call uart read
    return uart_read(file, ptr, len);
}

/** @brief _exit exits from the current user program by printing the
 *  status, then going into sleep mode. */
void _exit(int status) {
    
    char buffer[50];
    // Format the exit status into the buffer
    int length = snprintf(buffer, sizeof(buffer), "Program exited with status: %d\n", status);

    // If snprintf succeeds, length will be the number of characters generated, excluding the null byte
    if (length > 0) {
        // Send the formatted string via UART
        uart_write(FD_STDOUT, buffer, length);
    }

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
