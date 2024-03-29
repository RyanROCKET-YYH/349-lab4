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
#include <lcd_driver.h>
#include <keypad_driver.h>
#include <stdlib.h>
#include <servo.h>
#include <stdbool.h>
#include <i2c.h>
// #include <printk.h>

volatile bool isInCommandMode = true;

static void vHelloWorldTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        // when it is command mode, it doesn't print
        if (!isInCommandMode){
            printf("Hello World\n");
        }
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

    for (;;) {
        // only work when command mode
        if (isInCommandMode){
            write(STDOUT_FILENO, "> ", 2);
            // Attempt to read data from UART
            memset(buffer, 0, sizeof(buffer)); // clear buffer
            ssize_t numBytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            // check if data was read
            if (numBytesRead > 0) {
                //ensure the string is null-terminated
                buffer[numBytesRead] = '\0';
                // Echo back the received data
                write(STDOUT_FILENO, "You typed:", strlen("You typed:"));
                write(STDOUT_FILENO, buffer, numBytesRead);
                write(STDOUT_FILENO, "\n", 1);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * key_display():
 * @brief set the lcd cursor position.
*/
void key_display(char key, uint8_t *row, uint8_t *col) {
    if (*col >= 16) {
        *col = 0;
        if (*row == 0) {
            *row = 1;
        } else {
            lcd_clear();
            *row = 0; // Reset to first line
        }
    }

    lcd_set_cursor(*row, *col);
    char display[2] = {key, '\0'}; // Prepare the string for display
    lcd_print(display); // Show the key pressed

    (*col)++; // Move cursor position forward
}


void vKeypadServoLCDTask(void *pvParameters) {
    (void)pvParameters;
    char prompt[] = "Enter angle:";
    char angleStr[5] = {0};
    int angleIndex = 0;
    lcd_driver_init();

    // SERVO 1 (A0)
    gpio_init(GPIO_A, 0, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    // SERVO 2 (A1)
    gpio_init(GPIO_A, 1, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    servo_enable(0, 1);
    lcd_clear();
    lcd_print(prompt);

    for (;;) {
        // when it is command mode, it doesn't read keypad
        // if (!isInCommandMode){
        char key = keypad_read();
        if (key != '\0') {
            if ((key >= '0' && key <= '9') && angleIndex < 3) {
                angleStr[angleIndex++] = key;
                angleStr[angleIndex] = '\0';
                
                lcd_set_cursor(1, 0);
                lcd_print("                "); // Clear the second line by printing spaces
                lcd_set_cursor(1, 0);
                lcd_print(angleStr);

            } else if (key == '#') {
                angleStr[angleIndex] = '\0';
                int angle = atoi(angleStr);
                if (angle >= 0 && angle <= 180) {
                    servo_set(0, angle);
                } 
                
                memset(angleStr, 0, sizeof(angleStr)); // Clear angle string
                angleIndex = 0;
                lcd_set_cursor(1, 0);
                lcd_print("                ");
            }
        } 
        // }
        vTaskDelay(pdMS_TO_TICKS(100)); // Polling delay
    }
}

// 8.4
// void escapeSequenceTask(void *pvParameters) {
//     char byte;
//     while (1) {
//         if (!isInCommandMode) {
//             if (uart_get_byte(&byte)) { // If a byte was read
//                 if (atcmd_detect_escape(NULL, byte)) {
//                 }
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }



int main( void ) {
    uart_init(115200);
    keypad_init();
    i2c_master_init(80);

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

    // servo keypad
    xTaskCreate(
        vKeypadServoLCDTask, 
        "KeypadServoLCD", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;);
    return 0;
}
