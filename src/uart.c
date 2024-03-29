/**
 * @file   uart.c
 *
 * @brief  Interrupt and Console I/O
 *
 * @date   March 3rd
 *
 * @author Yuhong Yao (yuhongy), Yiying Li (yiyingl4)
 */

#include "FreeRTOS.h"
#include "task.h"
#include <unistd.h>
#include <rcc.h>
#include <uart.h>
#include <uart_polling.h>
#include <nvic.h>
#include <gpio.h>

/** @brief define UNUSE for unuse parameters */
#define UNUSED __attribute__((unused))

/** @brief set the buffer size */
#define BUFFER_SIZE (32)

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
#define UART2_BASE      (struct uart_reg_map *) 0x40004400

/** @brief Enable Bit for UART Config register */
#define UART_EN         (1 << 13)

/** @brief Pre calculated UARTDIV value for desired band rate of 115200 bps by default */
#define UARTDIV         0x8B

/** @brief Enable Bit for Transmitter */
#define UART_TE         (1 << 3)

/** @brief Enable Bit for Receiver */
#define UART_RE         (1 << 2)

/** @brief Transmit data register empty */
#define UART_SR_TXE     (1 << 7)

/** @brief Read data registter not empty */
#define UART_SR_RXNE    (1 << 5)

/** @brief set the RXNEIE bit of CR1 in URAT. */
#define UART_CR1_RXNEIE (1 << 5)

/** @brief set the TXEIE bit of CR1 in URAT. */
#define UART_CR1_TXEIE  (1 << 7)

/** @brief set the TCIE bit of CR1 in URAT. */
#define UART_CR1_TCIE   (1 << 6)

/** @brief set the number of IRQ in URAT. */
#define UART_IRQ_NUMBER (38)

/** @brief define Ring buffer */
typedef struct {
    /** @brief Ring buffer size*/
    unsigned char buffer[BUFFER_SIZE];
    /** @brief define Ring buffer head */
    volatile uint16_t head;
    /** @brief define Ring buffer tail */
    volatile uint16_t tail;
} RingBuffer;

/** @brief define txBuffer. */
RingBuffer txBuffer;
/** @brief define rxBuffer;. */
RingBuffer rxBuffer;

/** @brief initialize ring buffer as empty by setting both head and tail to 0. */
void RingBuffer_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
}

/** @brief check if the ring buffer is empty, if head equal to tail then it can be considered as empty. */
int RingBuffer_isEmpty(RingBuffer *rb) {
    return rb->head == rb->tail;
}

/** @brief check if buffer is full, since mod or divid will take a lot cycles, use masks to determine if full when tail wrapped to head. */
int RingBuffer_isFull(RingBuffer *rb) {
    
    return (((rb->tail + 1) & (BUFFER_SIZE - 1)) == rb->head); 
}

/** @brief add a byte to the buffer at tail. */
int RingBuffer_Write(RingBuffer *rb, char data) {
    int status;

    // taskENTER_CRITICAL();
    if (!RingBuffer_isFull(rb)) {
        rb->buffer[rb->tail] = data;
        rb->tail = (rb->tail + 1) & (BUFFER_SIZE - 1);
        status = 0; // Success
    } else {
        status = -1; // Buffer is full
    }
    // taskEXIT_CRITICAL(); // Exit the critical section

    return status;
}

/** @brief read the buffer from head and advance the head. */
int RingBuffer_Read(RingBuffer *rb, char *data) {
    int status;

    // taskENTER_CRITICAL();
    if (!RingBuffer_isEmpty(rb)) {
        *data = rb->buffer[rb->head];
        rb->head = (rb->head + 1) & (BUFFER_SIZE - 1);
        status = 0; // Success
    } else {
        status = -1; // Buffer is empty
    }
    // taskEXIT_CRITICAL(); // Exit the critical section

    return status;
}


/**
 * @brief uart_init: UART initialization function
 * baud  - baud rate
 */
void uart_init(UNUSED int baud) {
    //init ring buffer
    RingBuffer_init(&txBuffer);
    RingBuffer_init(&rxBuffer);

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
    nvic_irq(UART_IRQ_NUMBER, IRQ_ENABLE);
    uart->CR1 |= (UART_TE | UART_RE | UART_EN | UART_CR1_RXNEIE);
    return;
}

/**
 * @brief uart_put_byte: transmits a byte over UART
 * c  - character to be sent
 */
int uart_put_byte(UNUSED char c) {
    struct uart_reg_map *uart = UART2_BASE;
    int status = RingBuffer_Write(&txBuffer, c);
    uart->CR1 |= UART_CR1_TXEIE;
    return status;
}

/**
 * @brief uart_get_byte: receives a byte over UART
 * c  - character to be sent
 */
int uart_get_byte(UNUSED char *c) {
    char data;
    int status = RingBuffer_Read(&rxBuffer, &data);
    if (status == 0) {
        *c = data;
        return 0;
    }
    return status;
}

/**
 * @brief uart_write: support writing to stdout and return −1 if this is not the case
 * 
 */
int uart_write(UNUSED int file, UNUSED char *ptr, UNUSED int len) {
    if (file != STDOUT_FILENO) {
        return -1;
    }

    for (int i = 0; i < len; i++) {
        if (uart_put_byte(ptr[i]) == -1) {
            return -1;
        }
    }
    return len;
}

/**
 * @brief uart_read: support reading from stdin and return −1 if this is not the case
 * 
 */
int uart_read(UNUSED int file, char *ptr, int len) {
    if (file != STDIN_FILENO) {
        return -1;
    }
    
    int byteRead = 0;
    char c;
    while (1) {  // Keep reading until we get a newline
        if (uart_get_byte(&c) != 0) {
            continue;  // If no byte was read, try again
        }
        
        if (c == 4) {
            break;
        } else if (c == '\b') {
            if (byteRead > 0) {
                byteRead--;
                uart_write(STDOUT_FILENO, "\b \b", 3);
            }
        } else if (c == '\r' || c == '\n') {
            ptr[byteRead++] = '\n';
            uart_write(STDOUT_FILENO, "\r\n", 2);
            break;
        } else {
            if (byteRead < len - 1) {  // Make sure we don't overflow the buffer
                ptr[byteRead++] = c;
                uart_write(STDOUT_FILENO, &c, 1);
            }
        }
    }
    ptr[byteRead] = '\0';  // Null-terminate the string
    return byteRead;
}


/**
 * @brief uart_irq_handler: to handle the interrupt request (both receive and transmit)
 * 
 */
void uart_irq_handler() {
    struct uart_reg_map *uart = UART2_BASE;
    int transmitCount = 0;
    int receiveCount = 0;

    // Handle Transmission
    while ((uart->SR & UART_SR_TXE) && (transmitCount < 16)) {
        if (!RingBuffer_isEmpty(&txBuffer)) {
            char data;
            if (RingBuffer_Read(&txBuffer, &data) == 0) {
                uart->DR = data;
                transmitCount++;
            }
        } else {
            uart->CR1 &= ~UART_CR1_TXEIE;
            break;
        }
    }

    // Handle Reception
    while ((uart->SR & UART_SR_RXNE) && (receiveCount < 16)) {
        char data = uart->DR; // Reading DR clears the RXNE flag
        if (!RingBuffer_isFull(&rxBuffer)) {
            RingBuffer_Write(&rxBuffer, data);
            receiveCount++;
        } else {
            // Buffer is full, can't receive more data, break the loop
            break;
        }
    }

    nvic_clear_pending(UART_IRQ_NUMBER);
}
