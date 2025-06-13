#include "encoder.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdlib.h>
#include "hardware/irq.h"

// Global encoder state
static encoder_state_t encoder_state = {0};

// GPIO interrupt handler
void gpio_callback(uint gpio, uint32_t events) {
    // Handle X-axis encoder interrupts
    if (gpio == ENCODER_X1_PIN || gpio == ENCODER_X0_PIN) {
        encoder_x_interrupt_handler(gpio, events);
    }
    // Handle Y-axis encoder interrupts
    else if (gpio == ENCODER_Y1_PIN || gpio == ENCODER_Y0_PIN) {
        encoder_y_interrupt_handler(gpio, events);
    }
}

void encoder_init(void) {
    // Initialize GPIO pins for encoders
    gpio_init(ENCODER_X1_PIN);
    gpio_init(ENCODER_X0_PIN);
    gpio_init(ENCODER_Y1_PIN);
    gpio_init(ENCODER_Y0_PIN);
    
    // Set as inputs with pull-ups
    gpio_set_dir(ENCODER_X1_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_X0_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_Y1_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_Y0_PIN, GPIO_IN);
    
    gpio_pull_up(ENCODER_X1_PIN);
    gpio_pull_up(ENCODER_X0_PIN);
    gpio_pull_up(ENCODER_Y1_PIN);
    gpio_pull_up(ENCODER_Y0_PIN);
    
    // Set up interrupts for both edges on encoder A pins
    gpio_set_irq_enabled_with_callback(ENCODER_X1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(ENCODER_Y1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    
    // Initialize state
    encoder_state.x_position = 0;
    encoder_state.y_position = 0;
    encoder_state.x_last_state = 0;
    encoder_state.y_last_state = 0;
    encoder_state.initialized = true;
    
    // Read initial states
    encoder_state.x_last_state = (gpio_get(ENCODER_X1_PIN) << 1) | gpio_get(ENCODER_X0_PIN);
    encoder_state.y_last_state = (gpio_get(ENCODER_Y1_PIN) << 1) | gpio_get(ENCODER_Y0_PIN);
}

void encoder_reset(void) {
    encoder_state.x_position = 0;
    encoder_state.y_position = 0;
}

void encoder_set_center(void) {
    // Reset position to center
    encoder_reset();
}

int8_t encoder_get_x(void) {
    if (!encoder_state.initialized) {
        return 0;
    }
    
    // Scale encoder position to N64 controller range
    int32_t scaled_x = encoder_state.x_position * ENCODER_SCALE_FACTOR;
    
    // Apply deadzone
    if (abs(scaled_x) < STICK_DEADZONE) {
        scaled_x = 0;
    }
    
    // Clamp to controller range
    if (scaled_x > STICK_MAX_VALUE) scaled_x = STICK_MAX_VALUE;
    if (scaled_x < STICK_MIN_VALUE) scaled_x = STICK_MIN_VALUE;
    
    return (int8_t)scaled_x;
}

int8_t encoder_get_y(void) {
    if (!encoder_state.initialized) {
        return 0;
    }
    
    // Scale encoder position to N64 controller range
    int32_t scaled_y = encoder_state.y_position * ENCODER_SCALE_FACTOR;
    
    // Apply deadzone
    if (abs(scaled_y) < STICK_DEADZONE) {
        scaled_y = 0;
    }
    
    // Clamp to controller range
    if (scaled_y > STICK_MAX_VALUE) scaled_y = STICK_MAX_VALUE;
    if (scaled_y < STICK_MIN_VALUE) scaled_y = STICK_MIN_VALUE;
    
    return (int8_t)scaled_y;
}

void encoder_x_interrupt_handler(uint gpio, uint32_t events) {
    // Read current state of both X encoder pins
    uint32_t current_state = (gpio_get(ENCODER_X1_PIN) << 1) | gpio_get(ENCODER_X0_PIN);
    uint32_t last_state = encoder_state.x_last_state;
    
    // Quadrature decoding using state transition table
    // Forward: 00 -> 01 -> 11 -> 10 -> 00
    // Reverse: 00 -> 10 -> 11 -> 01 -> 00
    
    int8_t direction = 0;
    
    // State transition lookup table for quadrature decoding
    // [last_state][current_state] = direction
    static const int8_t quadrature_table[4][4] = {
        // 00, 01, 10, 11
        { 0, -1,  1,  0},  // from 00
        { 1,  0,  0, -1},  // from 01
        {-1,  0,  0,  1},  // from 10
        { 0,  1, -1,  0}   // from 11
    };
    
    direction = quadrature_table[last_state][current_state];
    encoder_state.x_position += direction;
    encoder_state.x_last_state = current_state;
}

void encoder_y_interrupt_handler(uint gpio, uint32_t events) {
    // Read current state of both Y encoder pins
    uint32_t current_state = (gpio_get(ENCODER_Y1_PIN) << 1) | gpio_get(ENCODER_Y0_PIN);
    uint32_t last_state = encoder_state.y_last_state;
    
    // Quadrature decoding using state transition table
    int8_t direction = 0;
    
    // State transition lookup table for quadrature decoding
    static const int8_t quadrature_table[4][4] = {
        // 00, 01, 10, 11
        { 0, -1,  1,  0},  // from 00
        { 1,  0,  0, -1},  // from 01
        {-1,  0,  0,  1},  // from 10
        { 0,  1, -1,  0}   // from 11
    };
    
    direction = quadrature_table[last_state][current_state];
    encoder_state.y_position += direction;
    encoder_state.y_last_state = current_state;
} 