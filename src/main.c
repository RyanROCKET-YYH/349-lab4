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
#include <atcmd.h>

// extern void initCommandParser(atcmd_parser_t *parser);
atcmd_parser_t parser;

uint8_t handleResume(void *args, const char *cmdArgs) {
    (void)args;
    (void)cmdArgs;
    isInCommandMode = false;
    printf("Exit command mode\n");
    // TODO: can't make it print "hello world" from Section 6.3 to resume printing.
    return 1; // sucess
}

uint8_t handleHello(void *args, const char *cmdArgs) {
    (void)args;
    if (cmdArgs != NULL) {
        printf("Hello, %s!\n", cmdArgs);
    } else {
        printf("Hello!\n"); //if <name> not provided
    }
    return 1; // success
}

uint8_t handlePasscode(void *args, const char *cmdArgs) {
    (void)args;
    (void)cmdArgs;
    //TODO: need to print passcode
    printf("handlePasscode\n");
    return 1; // success
}

uint8_t handlePasscodeChange(void *args, const char *cmdArgs) {
    (void)args;
    int passcode = atoi(cmdArgs);
    //TODO: need to change passcode
    printf("Passcode change to %d.\n", passcode); // TODO: can't make it print out the whole sentence
    return 1; // success
}

const atcmd_t commands[] = {
    {"RESUME", handleResume, NULL},
    {"HELLO", handleHello, NULL},
    {"PASSCODE?", handlePasscode, NULL},
    {"PASSCODE", handlePasscodeChange, NULL}
};

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
    // char testCmd2[] = "AT+RESUME";

    for (;;) {
        // only work when command mode
        if (isInCommandMode){
            write(STDOUT_FILENO, "> ", 2);
            // Attempt to read data from UART
            memset(buffer, 0, sizeof(buffer)); // clear buffer
            ssize_t numBytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            // check if data was read
            if (numBytesRead > 0) {
                buffer[numBytesRead] = '\0'; // Null-terminate the received string

                // Trim newline and carriage return from the end of the command
                for (int i = 0; i < numBytesRead; ++i) {
                    if (buffer[i] == '\n' || buffer[i] == '\r') {
                        buffer[i] = '\0';
                        break; // Stop at the first newline/carriage return character
                    }
                }
                // Now pass the trimmed and null-terminated command string to atcmd_parse
                atcmd_parse(&parser, buffer);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
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
        if (!isInCommandMode){
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
        }
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
    atcmd_parser_init(&parser, commands, (sizeof(commands) / sizeof(commands[0])));

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
    for(;;) {}
    return 0;
}
