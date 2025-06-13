#include "buttons.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Button pin mapping
static const uint button_pins[] = {
    BUTTON_A_PIN,     // A button
    BUTTON_B_PIN,     // B button
    BUTTON_Z_PIN,     // Z button
    BUTTON_START_PIN, // Start button
    BUTTON_DU_PIN,    // D-pad Up
    BUTTON_DD_PIN,    // D-pad Down
    BUTTON_DL_PIN,    // D-pad Left
    BUTTON_DR_PIN,    // D-pad Right
    BUTTON_L_PIN,     // L trigger
    BUTTON_R_PIN,     // R trigger
    BUTTON_CU_PIN,    // C-Up
    BUTTON_CD_PIN,    // C-Down
    BUTTON_CL_PIN,    // C-Left
    BUTTON_CR_PIN     // C-Right
};

static const uint16_t button_masks[] = {
    N64_BUTTON_A,
    N64_BUTTON_B,
    N64_BUTTON_Z,
    N64_BUTTON_START,
    N64_BUTTON_DU,
    N64_BUTTON_DD,
    N64_BUTTON_DL,
    N64_BUTTON_DR,
    N64_BUTTON_L,
    N64_BUTTON_R,
    N64_BUTTON_CU,
    N64_BUTTON_CD,
    N64_BUTTON_CL,
    N64_BUTTON_CR
};

#define NUM_BUTTONS (sizeof(button_pins) / sizeof(button_pins[0]))

// Debouncing state
static uint16_t button_state = 0;
static uint16_t button_state_prev = 0;
static uint32_t debounce_timer = 0;

#define DEBOUNCE_TIME_MS 5

void buttons_init(void) {
    // Initialize all button pins
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]); // Buttons pull to ground when pressed
    }
    
    button_state = 0;
    button_state_prev = 0;
    debounce_timer = to_ms_since_boot(get_absolute_time());
}

uint16_t buttons_read(void) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Only update button state if debounce period has passed
    if (current_time - debounce_timer >= DEBOUNCE_TIME_MS) {
        uint16_t new_state = 0;
        
        // Read all button pins
        for (int i = 0; i < NUM_BUTTONS; i++) {
            // Button is pressed when pin reads low (active low with pull-up)
            if (!gpio_get(button_pins[i])) {
                new_state |= button_masks[i];
            }
        }
        
        // Only update if state has changed
        if (new_state != button_state_prev) {
            button_state = new_state;
            button_state_prev = new_state;
            debounce_timer = current_time;
        }
    }
    
    return button_state;
}

bool buttons_is_reset_pressed(void) {
    uint16_t current_buttons = buttons_read();
    return (current_buttons & N64_RESET_MASK) == N64_RESET_MASK;
} 