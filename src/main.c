/**
 * @file main.c
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <uart.h>

int main( void ) {
    uart_init(115200);
    printf("\nHello world!\n");
    return 0;
}
