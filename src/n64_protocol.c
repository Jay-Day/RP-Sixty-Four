#include "n64_protocol.h"
#include "config.h"
#include "controller_pak.h"
#include "buttons.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "n64_protocol.pio.h"
#include <string.h>

// Global protocol state
static n64_controller_state_t current_state = {0};
static n64_controller_info_t controller_info = {
    .id_high = N64_CONTROLLER_ID_HIGH,
    .id_low = N64_CONTROLLER_ID_LOW,
    .status = 0
};

static uint n64_tx_offset;
static uint n64_rx_offset;
static volatile bool command_received = false;
static volatile uint8_t last_command = 0;

// CRC calculation for controller pak operations
uint8_t calculate_crc(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x85;  // N64 CRC polynomial
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Address checksum calculation for controller pak
uint8_t calculate_address_checksum(uint16_t address) {
    static const uint8_t checksum_table[] = {
        0x01, 0x1A, 0x0D, 0x1C, 0x0E, 0x07, 0x19, 0x16, 0x0B, 0x1F, 0x15
    };
    
    uint8_t checksum = 0;
    for (int i = 10; i >= 0; i--) {
        if (address & (1 << (15 - i))) {
            checksum ^= checksum_table[i];
        }
    }
    return checksum;
}

bool n64_protocol_init(void) {
    // Initialize GPIO pin
    gpio_init(N64_DATA_PIN);
    gpio_set_dir(N64_DATA_PIN, GPIO_OUT);
    gpio_put(N64_DATA_PIN, 1); // Idle high
    
    // Load PIO programs
    n64_tx_offset = pio_add_program(N64_PIO, &n64_tx_program);
    n64_rx_offset = pio_add_program(N64_PIO, &n64_rx_program);
    
    // Initialize PIO state machines
    n64_tx_program_init(N64_PIO, N64_PIO_SM, n64_tx_offset, N64_DATA_PIN);
    
    // Enable the state machine
    pio_sm_set_enabled(N64_PIO, N64_PIO_SM, true);
    
    return true;
}

void n64_protocol_update_state(const n64_controller_state_t* state) {
    if (state) {
        current_state = *state;
    }
}

void n64_protocol_reset(void) {
    // Clear any error flags
    controller_info.status &= ~N64_STATUS_CRC_ERROR;
    
    // Reset controller state if needed
    // The encoder module handles stick centering
}

void n64_send_byte(uint8_t data) {
    // Send 8 bits using PIO
    for (int i = 7; i >= 0; i--) {
        uint32_t bit = (data >> i) & 1;
        pio_sm_put_blocking(N64_PIO, N64_PIO_SM, bit);
    }
}

void n64_send_stop_bit(void) {
    // Send controller stop bit pattern (2μs low, 1μs high)
    pio_sm_exec_wait_blocking(N64_PIO, N64_PIO_SM, pio_encode_jmp(n64_tx_offset + n64_tx_offset_send_stop));
}

uint8_t n64_receive_byte(void) {
    uint8_t data = 0;
    // Simple bit-banged receive (could be improved with PIO)
    
    // Wait for start bit (falling edge)
    while (gpio_get(N64_DATA_PIN));
    
    // Sample 8 bits
    for (int i = 7; i >= 0; i--) {
        // Wait for middle of bit period
        busy_wait_us(2); // Sample at 2μs into bit period
        
        if (gpio_get(N64_DATA_PIN)) {
            data |= (1 << i);
        }
        
        // Wait for next bit
        busy_wait_us(2);
    }
    
    return data;
}

void n64_handle_info_command(void) {
    // Send controller ID and status
    n64_send_byte(controller_info.id_high);
    n64_send_byte(controller_info.id_low);
    n64_send_byte(controller_info.status);
    n64_send_stop_bit();
}

void n64_handle_poll_command(void) {
    // Send 4 bytes: button state (2 bytes) + stick X + stick Y
    uint16_t buttons = current_state.buttons;
    
    // Handle reset condition
    if ((buttons & (N64_BUTTON_L | N64_BUTTON_R | N64_BUTTON_START)) == 
        (N64_BUTTON_L | N64_BUTTON_R | N64_BUTTON_START)) {
        buttons |= 0x8000; // Set reset bit
        buttons &= ~N64_BUTTON_START; // Clear start bit when reset is active
    }
    
    // Send button data (MSB first)
    n64_send_byte((buttons >> 8) & 0xFF);
    n64_send_byte(buttons & 0xFF);
    
    // Send stick data
    n64_send_byte((uint8_t)current_state.stick_x);
    n64_send_byte((uint8_t)current_state.stick_y);
    
    n64_send_stop_bit();
}

void n64_handle_read_command(void) {
    // Receive 2-byte address with checksum
    uint16_t address_with_checksum = (n64_receive_byte() << 8) | n64_receive_byte();
    uint16_t address = address_with_checksum & 0xFFE0; // Mask off checksum bits
    uint8_t received_checksum = address_with_checksum & 0x1F;
    uint8_t calculated_checksum = calculate_address_checksum(address);
    
    uint8_t response_data[32];
    uint8_t crc;
    
    if (received_checksum == calculated_checksum) {
        // Read from controller pak
        controller_pak_read(address, response_data, 32);
        crc = calculate_crc(response_data, 32);
    } else {
        // Invalid checksum - return zeros
        memset(response_data, 0, 32);
        crc = 0xFF;
        controller_info.status |= N64_STATUS_CRC_ERROR;
    }
    
    // Send 32 bytes of data
    for (int i = 0; i < 32; i++) {
        n64_send_byte(response_data[i]);
    }
    
    // Send CRC
    n64_send_byte(crc);
    n64_send_stop_bit();
}

void n64_handle_write_command(void) {
    // Receive 2-byte address with checksum
    uint16_t address_with_checksum = (n64_receive_byte() << 8) | n64_receive_byte();
    uint16_t address = address_with_checksum & 0xFFE0;
    uint8_t received_checksum = address_with_checksum & 0x1F;
    uint8_t calculated_checksum = calculate_address_checksum(address);
    
    // Receive 32 bytes of data
    uint8_t write_data[32];
    for (int i = 0; i < 32; i++) {
        write_data[i] = n64_receive_byte();
    }
    
    uint8_t crc = calculate_crc(write_data, 32);
    
    if (received_checksum == calculated_checksum) {
        // Write to controller pak
        controller_pak_write(address, write_data, 32);
    } else {
        controller_info.status |= N64_STATUS_CRC_ERROR;
    }
    
    // Apply pak removed flag to CRC if needed
    if (controller_info.status & N64_STATUS_PAK_REMOVED) {
        crc ^= 0xFF;
    }
    
    // Send CRC response
    n64_send_byte(crc);
    n64_send_stop_bit();
}

void n64_handle_command(uint8_t command) {
    switch (command) {
        case N64_CMD_INFO:
            n64_handle_info_command();
            break;
            
        case N64_CMD_POLL:
            n64_handle_poll_command();
            break;
            
        case N64_CMD_READ:
            n64_handle_read_command();
            break;
            
        case N64_CMD_WRITE:
            n64_handle_write_command();
            break;
            
        case N64_CMD_RESET:
            n64_protocol_reset();
            n64_handle_info_command(); // Send info after reset
            break;
            
        default:
            // Unknown command - send info as default
            n64_handle_info_command();
            break;
    }
}

void n64_protocol_task(void) {
    // Set pin as input to listen for commands
    gpio_set_dir(N64_DATA_PIN, GPIO_IN);
    gpio_pull_up(N64_DATA_PIN);
    
    // Wait for falling edge (start of command)
    while (gpio_get(N64_DATA_PIN));
    
    // Receive command byte
    uint8_t command = n64_receive_byte();
    
    // Switch to output mode for response
    gpio_set_dir(N64_DATA_PIN, GPIO_OUT);
    
    // Small delay before response
    busy_wait_us(1);
    
    // Handle the command
    n64_handle_command(command);
    
    // Return to idle state
    gpio_put(N64_DATA_PIN, 1);
} 