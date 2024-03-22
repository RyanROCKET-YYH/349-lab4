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
#include <gpio.h>
#include <unistd.h>
#include <string.h>
// #include <printk.h>

static void vHelloWorldTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        printf("Hello World\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vBlinkyTask(void *pvParameters) {
    (void)pvParameters;
    // blue led init
    gpio_init(GPIO_A, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);

    for(;;) {
        if (gpio_read(GPIO_A, 5)) {
            gpio_clr(GPIO_A, 5);
        } else {
            gpio_set(GPIO_A, 5);
        }

        // wait for 1000 millisec
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
}

static void vUARTEchoTask(void *pvParameters) {
    (void)pvParameters;
    char buffer[100];
    ssize_t numBytesRead;

    for (;;) {
        // Attempt to read data from UART
        numBytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        // check if data was read
        if (numBytesRead > 0) {
            //ensure the string is null-terminated
            buffer[numBytesRead] = '\0';
            // Echo back the received data
            write(STDOUT_FILENO, "You typed:", strlen("You typed:"));
            write(STDOUT_FILENO, buffer, numBytesRead);
            write(STDOUT_FILENO, "\n", 1);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


int main( void ) {
    uart_init(115200);

    xTaskCreate(
        vBlinkyTask,
        "BlinkyTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    // Create the UART echo task
    xTaskCreate(
        vUARTEchoTask,
        "UARTEcho",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL); 

     xTaskCreate(
        vHelloWorldTask, 
        "HelloWorld",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;);
    return 0;
}
