/* lcd_driver.c contains functions of initilaizing and setting the lcd. */

#include "FreeRTOS.h"
#include "task.h"
#include <i2c.h>
#include <lcd_driver.h>
#include <unistd.h>
// #include <systick.h>


#define I2C_SLAVE_ADDR_W 0x4E   // 7 bit address
#define I2C_SLAVE_ADDR_R 0x4F

/*
 * lcd_send_instruction():
 * To send the instruction to lcd by i2c_write.
*/
void lcd_send_instruction(uint8_t command) {
    uint8_t LCD_ADDR = I2C_SLAVE_ADDR_W; 
    uint8_t i2c_write_buf[4];
    // (1=1, E = 1, RW=0, RS=0)
    taskENTER_CRITICAL(); 
    i2c_write_buf[0] = (command & 0xF0) | 0b1100;
    i2c_write_buf[1] = (command & 0xF0) | 0b1000;
    i2c_write_buf[2] = command << 4 | 0b1100;
    i2c_write_buf[3] = command << 4 | 0b1000;
    i2c_master_write(i2c_write_buf, 4, LCD_ADDR);
    taskEXIT_CRITICAL();
}

/*
 * lcd_send_data():
 * To send the data to lcd by i2c_write.
*/
void lcd_send_data(uint8_t data) {
    uint8_t LCD_ADDR = I2C_SLAVE_ADDR_W; 
    uint8_t i2c_write_buf[4];
    // (1=1, E = 1, RW=0, RS=1)
    taskENTER_CRITICAL();
    i2c_write_buf[0] = (data & 0xF0) | 0b1101;
    i2c_write_buf[1] = (data & 0xF0) | 0b1001;
    i2c_write_buf[2] = data << 4 | 0b1101;
    i2c_write_buf[3] = data << 4 | 0b1001;
    i2c_master_write(i2c_write_buf, 4, LCD_ADDR);
    taskEXIT_CRITICAL();
}

/*
 * lcd_driver_init():
 * To initialize the lcd_driver.
*/
void lcd_driver_init(){
    // wait for 15ms
    vTaskDelay(pdMS_TO_TICKS(15));
    lcd_send_instruction(0b00110000);
    
    // wait for 5 ms
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_send_instruction(0b00110000);

    // wait for 1 ms
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_instruction(0b00110000);

    lcd_send_instruction(0b00100000);  // Function set (set interface to 4 bits long)
    
    // clear display
    lcd_send_instruction(0b00000001);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/*
 * lcd_print():
 * To print the data from input to lcd.
*/
void lcd_print(char *input){
    while (*input) {
        lcd_send_data((uint8_t)(*input));
        input++;
    }
}

/*
 * lcd_set_cursor():
 * To set the cursor of lcd.
*/
void lcd_set_cursor(uint8_t row, uint8_t col){
    uint8_t address;
    switch(row) {
        case 0:
            address = 0x00 + col;
            break;
        case 1:
            address = 0x40 + col;
            break;
        default:
            address = 0x00;
    }
    lcd_send_instruction(0x80 | address);
}

/*
 * lcd_clear():
 * To clear the lcd.
*/
void lcd_clear(){
    lcd_send_instruction(0b00000001);
    //after clear instruction, wait for 2 sec

    vTaskDelay(pdMS_TO_TICKS(2000));
}
