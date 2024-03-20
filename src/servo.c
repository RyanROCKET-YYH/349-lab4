/**
 * @file servo.c
 *
 * @brief functions for servo
 *
 * @date 03/15/2024
 *
 * @author Yuhong Yao (yuhongy), Yiying Li (yiyingl4)
 */

#include <unistd.h>
#include <gpio.h>
#include <timer.h>
#include <nvic.h>
#include <printk.h>

/** @brief define UNUSE for unuse parameters */
#define UNUSED __attribute__((unused))

/** @brief macro for channel 0 pin */
#define CHANNEL0_PIN (0)
/** @brief macro for channel 1 pin */
#define CHANNEL1_PIN (1)
/** @brief macro for servo period */
#define SERVO_PERIOD (200)

/**
 * ServoChannel:
 * @brief Set the the parameters of servo
 */
typedef struct {
    /** @brief define high tick when enable servo */
    uint16_t high_tick;
    /** @brief define low tick */
    uint16_t low_tick;
    /** @brief define current tick */
    uint16_t current_tick;
    /** @brief define port */
    gpio_port port;
    /** @brief define gpio_pin */
    unsigned int gpio_pin;
    /** @brief define enabled */
    uint8_t enabled;
    /** @brief define is_high  */
    uint8_t is_high;
} ServoChannel;

/**
 * ServoChannel:
 * @brief Set the two servos parameters
 */
ServoChannel servos[2] = {
    {15, SERVO_PERIOD - 15, 0, GPIO_A, CHANNEL0_PIN, 0, 0},
    {15, SERVO_PERIOD - 15, 0, GPIO_A, CHANNEL1_PIN, 0, 0}
};

/**
 * ServoChannel:
 * @brief convert angle to period
 *
 */
uint16_t angle_to_tick(UNUSED uint8_t angle) {
    // 0 degree is 0.6 ms and 180 degree is 2.4ms period is 20ms (50hz)
    // prescalar is 1600, one tick every 0.1 ms, .. 6 tick for 0 and 24 tick for 180
    return (6 + (0.1 * angle));
}

/**
 * tim2_irq_handler():
 * @brief Set time 5 interrupt requestion handler
 *
 */
void tim2_irq_handler() {
    struct tim2_5* tim2 = timer_base[2];
    ServoChannel *s1 = &servos[0];
    if (tim2->sr & TIM_SR_UIF) {
        if (1) {  
            s1->current_tick++;
            if (s1->is_high) {
                if (s1->current_tick >= s1->high_tick) {
                    gpio_clr(GPIO_A, CHANNEL0_PIN);
                    s1->is_high = 0;
                    s1->current_tick = 0;
                }
            } else {
                if (s1->current_tick >= s1->low_tick) {
                    gpio_set(GPIO_A, CHANNEL0_PIN);
                    s1->is_high = 1;
                    s1->current_tick = 0;
                }
            }
        } 
        timer_clear_interrupt_bit(2);
    }
}

/**
 * tim5_irq_handler():
 * @brief Set time 5 interrupt requestion handler
 *
 */
void tim5_irq_handler() {
    struct tim2_5* tim5 = timer_base[5];
    ServoChannel *s2 = &servos[1];
    if (tim5->sr & TIM_SR_UIF) {
        if (s2->enabled) {
            s2->current_tick++;
            if (s2->is_high) {
                if (s2->current_tick >= s2->high_tick) {
                    gpio_clr(GPIO_A, CHANNEL1_PIN);
                    s2->is_high = 0;
                    s2->current_tick = 0;
                }
            } else {
                if (s2->current_tick >= s2->low_tick) {
                    gpio_set(GPIO_A, CHANNEL1_PIN);
                    s2->is_high = 1;
                    s2->current_tick = 0;
                }
            }
        } 
        timer_clear_interrupt_bit(5);
    }
}


/**
 * @brief Enable or disable servo motor control
 *
 * @param channel  channel to enable or disable
 * @param enabled  1 to enable, 0 to disable
 *
 * @return 0 on success or -1 on failure
 */
int servo_enable(UNUSED uint8_t channel, UNUSED uint8_t enabled){
    if (channel != 0 && channel != 1) {
        printk("Invalid Channel\n");
        return -1;
    }

    ServoChannel *sc = &servos[channel];
    sc->enabled = enabled;
    if (enabled) {
        if (channel == 0) {
            timer_init(2, 100, 16);
        } else {
            timer_init(5, 100, 16);
        }
        // when enabled, enable the periodic signal on the given channel
        if (!sc->is_high) {
            // gpio_set(sc->port, sc->gpio_pin);
            sc->is_high = 1;
            sc->current_tick = 0;
        }
    } else {
        if (channel == 0) {
            timer_disable(2);
        } else {
            timer_disable(5);
        }
        gpio_clr(sc->port, sc->gpio_pin);
        sc->is_high = 0;
        sc->current_tick = 0;   // reset the current tick
    }

    return 0;
}


/**
 * @brief Set a servo motor to a given position
 *
 * @param channel   channel to control
 * @param angle     servo angle in degrees (0-180)
 *
 * @return 0 on success or -1 on failure
 */
int servo_set(UNUSED uint8_t channel, UNUSED uint8_t angle){
    if (channel > 1 || angle > 180) return -1;
    ServoChannel *sc = &servos[channel];
    uint16_t pulse_width = angle_to_tick(angle);
    sc->high_tick = pulse_width;
    sc->low_tick = SERVO_PERIOD - pulse_width; 
    return 0;
}
