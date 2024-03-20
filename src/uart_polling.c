/* uart_polling.c contains functions of initilizing and setting the uart. */
#include <gpio.h>
#include <rcc.h>
#include <unistd.h>
#include <uart_polling.h>

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};

/** @brief Base address for UART2 */
#define UART2_BASE  (struct uart_reg_map *) 0x40004400

/** @brief Enable Bit for UART Config register */
#define UART_EN (1 << 13)

/** @brief Pre calculated UARTDIV value for desired band rate of 115200 bps by default */
#define UARTDIV 0x8B

/** @brief Enable Bit for Transmitter */
#define UART_TE (1 << 3)

/** @brief Enable Bit for Receiver */
#define UART_RE (1 << 2)

/** @brief Transmit data register empty */
#define UART_SR_TXE (1 << 7)

/** @brief Read data registter not empty */
#define UART_SR_RXNE (1 << 5)

/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    if (baud == 0) {
        return;
    }
    struct uart_reg_map *uart = UART2_BASE;
    // Reset and Clock Control
    struct rcc_reg_map *rcc = RCC_BASE;
    rcc->apb1_enr |= UART_CLKEN;
    // GPIO Pins
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);        /* PA_2 for TX line UART2 */
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);       /* PA_2 for RX line UART2 */
    
    // Initialize UART to the desired Baud Rate
    uart->BRR = UARTDIV;
    // UART Control Registers
    uart->CR1 |= (UART_TE | UART_RE | UART_EN);
    return;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){
    struct uart_reg_map *uart = UART2_BASE;
    // Wait the data register to be empty
    while (!((uart->SR) & UART_SR_TXE)){};
    // Once ready, write and return
    *(char *)&uart->DR = c;
    return;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte (){
    struct uart_reg_map *uart = UART2_BASE;
    // Wait until ready to be read
    while (!(uart->SR & UART_SR_RXNE)){};
    // return received data
    return (char)(uart->DR);
}
