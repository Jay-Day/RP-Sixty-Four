#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "config.h"
#include "n64_protocol.h"
#include "encoder.h"
#include "buttons.h"
#include "controller_pak.h"

// Global controller state
static n64_controller_state_t controller_state = {0};

// Status LED control
void status_led_init(void) {
    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_PIN, 0);
}

void status_led_set(bool on) {
    gpio_put(STATUS_LED_PIN, on);
}

// Status LED blink pattern for debugging
void status_led_blink(int count, int delay_ms) {
    for (int i = 0; i < count; i++) {
        status_led_set(true);
        sleep_ms(delay_ms);
        status_led_set(false);
        if (i < count - 1) {
            sleep_ms(delay_ms);
        }
    }
}

// Core 1 task - handles N64 protocol communication
void core1_task(void) {
    while (true) {
        // Handle N64 protocol communication
        n64_protocol_task();
        
        // Small delay to prevent busy waiting
        sleep_us(10);
    }
}

// Update controller state from inputs
void update_controller_state(void) {
    // Read button states
    controller_state.buttons = buttons_read();
    
    // Read encoder positions
    controller_state.stick_x = encoder_get_x();
    controller_state.stick_y = encoder_get_y();
    
    // Handle reset condition (L + R + Start pressed)
    if (buttons_is_reset_pressed()) {
        encoder_set_center(); // Reset stick to center
        status_led_blink(3, 100); // Indicate reset
    }
    
    // Update the protocol handler with new state
    n64_protocol_update_state(&controller_state);
}

// Initialize all subsystems
bool system_init(void) {
    // Initialize stdio
    stdio_init_all();
    
    #if DEBUG_ENABLE
    printf("RP2040 N64 Controller Emulator Starting...\n");
    #endif
    
    // Initialize status LED
    status_led_init();
    status_led_blink(1, 200); // Power-on indicator
    
    // Initialize encoder system
    encoder_init();
    #if DEBUG_ENABLE
    printf("Encoder system initialized\n");
    #endif
    
    // Initialize button system
    buttons_init();
    #if DEBUG_ENABLE
    printf("Button system initialized\n");
    #endif
    
    // Initialize controller pak emulation
    if (!controller_pak_init()) {
        #if DEBUG_ENABLE
        printf("Controller pak initialization failed\n");
        #endif
        status_led_blink(5, 100); // Error indicator
        return false;
    }
    #if DEBUG_ENABLE
    printf("Controller pak initialized\n");
    #endif
    
    // Initialize N64 protocol handler
    if (!n64_protocol_init()) {
        #if DEBUG_ENABLE
        printf("N64 protocol initialization failed\n");
        #endif
        status_led_blink(10, 100); // Error indicator
        return false;
    }
    #if DEBUG_ENABLE
    printf("N64 protocol initialized\n");
    #endif
    
    // Center the stick
    encoder_set_center();
    
    #if DEBUG_ENABLE
    printf("System initialization complete\n");
    printf("Waiting for N64 console commands...\n");
    #endif
    
    status_led_blink(2, 100); // Success indicator
    return true;
}

// Main program
int main(void) {
    // Initialize system
    if (!system_init()) {
        // Initialization failed - flash LED continuously
        while (true) {
            status_led_blink(1, 250);
            sleep_ms(750);
        }
    }
    
    // Launch N64 protocol handler on core 1
    multicore_launch_core1(core1_task);
    
    // Main loop on core 0 - handle input processing
    uint32_t last_update = 0;
    const uint32_t update_interval_us = 1000; // 1ms update rate (1000 Hz)
    
    while (true) {
        uint32_t current_time = time_us_32();
        
        // Update controller state at regular intervals
        if (current_time - last_update >= update_interval_us) {
            update_controller_state();
            last_update = current_time;
            
            // Heartbeat LED (slow blink during normal operation)
            static uint32_t led_timer = 0;
            if (current_time - led_timer > 1000000) { // 1 second
                status_led_set(true);
                sleep_us(50000); // 50ms pulse
                status_led_set(false);
                led_timer = current_time;
            }
        }
        
        #if DEBUG_ENABLE
        // Debug output every second
        static uint32_t debug_timer = 0;
        if (current_time - debug_timer > 1000000) {
            printf("Stick: X=%d, Y=%d, Buttons=0x%04X\n", 
                   controller_state.stick_x, 
                   controller_state.stick_y, 
                   controller_state.buttons);
            debug_timer = current_time;
        }
        #endif
        
        // Small delay to prevent busy waiting
        sleep_us(100);
    }
    
    return 0;
} 