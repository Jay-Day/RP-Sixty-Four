#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/types.h"

// Encoder state structure
typedef struct {
    volatile int32_t x_position;
    volatile int32_t y_position;
    volatile uint32_t x_last_state;
    volatile uint32_t y_last_state;
    bool initialized;
} encoder_state_t;

// Function prototypes
void encoder_init(void);
void encoder_reset(void);
int8_t encoder_get_x(void);
int8_t encoder_get_y(void);
void encoder_set_center(void);

// Internal interrupt handlers (called from GPIO IRQ)
void encoder_x_interrupt_handler(uint gpio, uint32_t events);
void encoder_y_interrupt_handler(uint gpio, uint32_t events);

#endif // ENCODER_H 