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

#define MAX_PASSCODE_LENGTH 12
#define LOCKED_POSITION 0
#define UNLOCKED_POSITION 180

atcmd_parser_t parser;
int g_passcode = 349;

uint8_t handleResume(void *args, const char *cmdArgs) {
    (void)args;
    (void)cmdArgs;
    isInCommandMode = false;
    printf("Exit command mode\n");
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
    printf("Current passcode: %d\n", g_passcode);
    return 1; // success
}

uint8_t handlePasscodeChange(void *args, const char *cmdArgs) {
    (void)args; 

    if (cmdArgs != NULL && strlen(cmdArgs) <= 12) {
        int new_passcode = atoi(cmdArgs);
        g_passcode = new_passcode;
        printf("Passcode changed successfully.\n");
        return 1; // Success
    } else if (cmdArgs == NULL) {
        printf("No passcode provided.\n");
    } else {
        printf("Invalid passcode (string up to 12 digits).\n");
    }
    return 0; // Fail
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
    char passcode[MAX_PASSCODE_LENGTH] = {0};
    char prompt[] = "Enter password:";
    uint8_t index = 0;
    uint8_t is_locked = 1;
    lcd_driver_init();

    // SERVO 1 (A0)
    gpio_init(GPIO_A, 0, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    // SERVO 2 (A1)
    gpio_init(GPIO_A, 1, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    servo_enable(0, 1);
    servo_set(0,LOCKED_POSITION); // initialized to lock state
    lcd_clear();
    lcd_print(prompt);

    for (;;) {
        // when it is command mode, it doesn't read keypad
        if (!isInCommandMode){
            char key = keypad_read();
            if (key != '\0') {
                if ((key >= '0' && key <= '9') && index < MAX_PASSCODE_LENGTH) {
                    passcode[index++] = key;
                    passcode[index] = '\0';
                    
                    lcd_set_cursor(1, 0);
                    lcd_print("                "); // Clear the second line by printing spaces
                    lcd_set_cursor(1, 0);
                    lcd_print(passcode);
                } else if (key == '#') {
                    passcode[index] = '\0';
                    int password = atoi(passcode);
                    if (password == g_passcode) {
                        is_locked = !is_locked;
                        servo_set(0, is_locked ? LOCKED_POSITION : UNLOCKED_POSITION);
                        lcd_clear();
                        lcd_print(is_locked ? "Locking!" : "Unlocking!");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        lcd_clear();
                        lcd_print(prompt);
                    } else {
                        lcd_clear();
                        lcd_print("Incorrect Passco");
                        lcd_set_cursor(1,0);
                        lcd_print("de...");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        lcd_clear();
                        lcd_print(prompt);
                    }
                    
                    memset(passcode, 0, sizeof(passcode)); // Clear angle string
                    index = 0;
                }
            } 
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Polling delay
    }
}

// 8.4
void escapeSequenceTask(void *pvParameters) {
    (void)pvParameters;
    char byte;
    while (1) {
        if (!isInCommandMode) {
            if (uart_get_byte(&byte) == 0) { 
                // Directly pass each byte to atcmd_detect_escape
                if (atcmd_detect_escape(NULL, byte)) {
                    isInCommandMode = 1;
                    printf("Entering Command Mode.\n");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}



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

    xTaskCreate(
        escapeSequenceTask, 
        "ENTERCommand", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL);
    vTaskStartScheduler();
    
    // Infinite loop
    for(;;) {}
    return 0;
}
