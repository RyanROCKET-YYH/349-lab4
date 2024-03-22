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
// #include <printk.h>

// static void vHelloWorldTask(void *pvParameters) {
//     (void)pvParameters;  // Explicitly mark the parameter as unused

//     uart_init(115200);  // Initialize UART at 115200 baud rate
//     for(;;) {
//         // printk("\nprintk speaking\n");
//         printf("\nHello world! printf\n");
//         vTaskDelay(pdMS_TO_TICKS(1000));  // Delay for 1 second
//     }
// }

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


int main( void ) {
    xTaskCreate(
        vBlinkyTask,
        "BlinkyTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;);
    return 0;
}
