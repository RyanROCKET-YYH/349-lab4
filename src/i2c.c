/**
 * @file i2c.c
 *
 * @brief i2c.c contains the functions to implement i2c master mode
 *
 * @date 03/29/2024
 *
 * @author Yuhong Yao (yuhongy), Yiying Li (yiyingl4)
 */
#include <gpio.h>
#include <i2c.h>
#include <unistd.h>
#include <rcc.h>


/** @brief The i2c register map. */
struct i2c_reg_map {
    volatile uint32_t CR1;      /**<  Control Register 1 */
    volatile uint32_t CR2;      /**<  Control Register 2 */
    volatile uint32_t OAR1;     /**<  Own Address Register 1 */
    volatile uint32_t OAR2;     /**<  Own Address Register 2 */
    volatile uint32_t DR;       /**<  Data Register */
    volatile uint32_t SR1;      /**<  Status Register 1 */
    volatile uint32_t SR2;      /**<  Status Register 2 */
    volatile uint32_t CCR;      /**<  Clock Control Register */
    volatile uint32_t TRISE;    /**<  TRISE Register */
    volatile uint32_t FLTR;     /**<  FLTR Register */
};

/** @brief Base Address of I2C1 */
#define I2C1_BASE   (struct i2c_reg_map *) 0x40005400

/** @brief Peripheral Clock Frequency(16 MHz) of I2C */
#define I2C_CF  0b10000

/** @brief I2C clock speed: 100kHz */
#define I2C_CCR  0x50

/** @brief Start bit mask */
#define I2C_CR1_START  (1 << 8)

/** @brief Stop bit mask */
#define I2C_CR1_STOP  (1 << 9)
/** @brief Enable mask */
#define I2C_EN  (1)
/** @brief I2C_CR1_SWRST bit mask */
#define I2C_CR1_SWRST (1 << 15)
/** @brief I2C_SR1_BTF bit mask */
#define I2C_SR1_BTF (1 << 2)
/** @brief I2C_SR1_TXE bit mask */
#define I2C_SR1_TXE (1 << 7)
/** @brief I2C_SR1_ADDR bit mask */
#define I2C_SR1_ADDR (1 << 1)
/** @brief I2C_TRISE bit mask */
#define I2C_TRISE (0x17)
/** @brief I2C_SR2_BUSY bit mask */
#define I2C_SR2_BUSY (1 << 1)
/** @brief I2C_CR1_ACK bit mask */
#define I2C_CR1_ACK (1 << 10)
/** @brief I2C_SR1_SB bit mask */
#define I2C_SR1_SB (1)

/**
 *
 * @brief  initialize the master mode in i2c.
 *
 */
void i2c_master_init(uint16_t clk){
    (void) clk;

    // set rcc
    struct i2c_reg_map *i2c = I2C1_BASE;
    struct rcc_reg_map *rcc = RCC_BASE;
    rcc->apb1_enr |= I2C1_CLKEN;

    // GPIO Pins(D14: I2C1_SDA, D15: I2C1_SCL)
    gpio_init(GPIO_B, 8, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT4);   /* PB_8(D15), SCL */
    gpio_init(GPIO_B, 9, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT4);   /* PB_9(D14), SDA */

    // Peripheral Clock Frequency: 16 Mhz
    i2c->CR2 |= I2C_CF;

    // enable CCR, peripheral, and ACK
    i2c->CCR |= I2C_CCR;
    i2c->CR1 |= I2C_EN;
    i2c->CR1 |= I2C_CR1_ACK;
    return;
}

/**
 *
 * @brief set the start condition.
 *
 */
int i2c_master_start(){
    struct i2c_reg_map *i2c = I2C1_BASE;
    // First, wait until BUSY bit is cleared
    while (i2c->SR2 & I2C_SR2_BUSY);
    // Then set the START bit to generate the start condition
    i2c->CR1 |= I2C_CR1_START; // Make sure I2C_SB is the correct mask for the START bit in CR1
    // Now wait for the SB flag to be set
    while (!(i2c->SR1 & I2C_SR1_SB));
    return 0;
}

/**
 *
 * @brief set the stop condition.
 *
 */
int i2c_master_stop(){
    struct i2c_reg_map *i2c = I2C1_BASE;
    // check TxE and BTF bit (they should be set to 1)(EV8_2)
    while (!(i2c->SR1 & I2C_SR1_BTF) && !(i2c-> SR1 & I2C_SR1_TXE));
    // set stop bit
    i2c->CR1 |= I2C_CR1_STOP;
    return 0;
}

/**
 *
 * @brief The main function of writing string that stored in buf to slave address.
 *
 */
int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    struct i2c_reg_map *i2c = I2C1_BASE;

    // start condition of the master transmission
    i2c_master_start();

    // write the slave address of slave into data register
    i2c->DR = slave_addr; 
    // wait for ADDR to be set
    while(!(i2c->SR1 & I2C_SR1_ADDR));  
    // Clear ADDR by reading SR2
    (void)i2c->SR2;

    // write buf into data register
    for(int i = 0; i < len; i++){
        // Wait for TxE to be set
        while (!(i2c->SR1 & I2C_SR1_TXE));
        // Send data byte
        i2c->DR = buf[i];
    }
    
    // Wait for BTF to be set before issuing stop condition
    while (!(i2c->SR1 & I2C_SR1_BTF));
    // Send stop condition
    i2c_master_stop();
    return 0;
}

/**
 *
 * @brief It is to debug the i2c_master_write().
 *
 */
int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    (void) buf;
    (void) len;
    (void) slave_addr;

    return 0;
}