/* keypad_driver.c contains functions of  initilaizing and reading the number of keypad. */

#include <gpio.h>
#include <keypad_driver.h>
#include <unistd.h>
// #include <systick.h>

// the variables of keypad(columns and rows)
#define COL1_PORT  0
#define COL1_PIN   7
#define COL2_PORT  2
#define COL2_PIN   0
#define COL3_PORT  2
#define COL3_PIN   7

#define ROW1_PORT  0
#define ROW1_PIN   6
#define ROW2_PORT  0
#define ROW2_PIN   8
#define ROW3_PORT  0
#define ROW3_PIN   9
#define ROW4_PORT  1
#define ROW4_PIN   6

#define NUM_ROWS   4
#define NUM_COLS   3

// Placeholder for actual GPIO port and pin numbers
const int row_pins[NUM_ROWS] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
const int col_pins[NUM_COLS] = {COL1_PIN, COL2_PIN, COL3_PIN};
const int row_ports[NUM_ROWS] = {ROW1_PORT, ROW2_PORT, ROW3_PORT, ROW4_PORT};
const int col_ports[NUM_COLS] = {GPIO_A, GPIO_C, GPIO_C};

const char key_map[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

/* 
 * keypad_init():
 * initialize the 7 pins of the keypad.
 * set the columns as output and rows as input.
 * set the columns as high.
*/
void keypad_init(){

    // columns as output
    gpio_init(GPIO_A, 7, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);		//pa_7
    gpio_init(GPIO_C, 0, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);		//pc_0
    gpio_init(GPIO_C, 7, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);		//pc_7

    //rows as input 
    gpio_init(GPIO_A, 6, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);		//pa_6
    gpio_init(GPIO_A, 8, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);		//pa_8
    gpio_init(GPIO_A, 9, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);		//pa_9
    gpio_init(GPIO_B, 6, MODE_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);		//pb_6
    
    gpio_set(GPIO_A, COL1_PIN);
    gpio_set(GPIO_A, COL2_PIN);
    gpio_set(GPIO_C, COL3_PIN);
    return;
}

/* 
 * keypad_read():
 * read what was pressed in keypad and return a char
*/
char keypad_read() {
    // debounce the inputs
    // const uint32_t debounce_time = 20;  // 20 ms
    // uint32_t last_debounce = 0;
    char key = '\0';
    for (int col = 0; col < NUM_COLS; col++) {
        // Set the current column to LO to enable to read from 7 pins
        gpio_clr(col_ports[col], col_pins[col]);
        for (int row = 0; row < NUM_ROWS; row++) {
            // Read the current row
            if (gpio_read(row_ports[row], row_pins[row]) == 0) {
                // if ((systick_get_ticks() - last_debounce) > debounce_time) {
                key = key_map[row][col]; // Store the detected key
                    // last_debounce = systick_get_ticks();
                // }
                // Wait for the key to be released
                while(gpio_read(row_ports[row], row_pins[row]) == 0) {
                    // last_debounce = systick_get_ticks();    // update debounce 
                }
                gpio_set(col_ports[col], col_pins[col]); // Set column back to HI
                return key; // Return the detected key
            }
        }

        // Set the column back to HI
        gpio_set(col_ports[col], col_pins[col]);
    }
    return '\0'; // Return '\0' if no key is pressed
}

