/**
 * @file
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#ifndef _TIMER_H_
#define _TIMER_H_

void timer_init(int timer, uint32_t prescalar, uint32_t period);

void timer_disable(int timer);

void timer_clear_interrupt_bit(int timer);

#endif /* _TIMER_H_ */
