#include "controller_pak.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

// Controller pak data in RAM
static uint8_t controller_pak_data[CONTROLLER_PAK_SIZE];
static bool pak_present = true;
static bool pak_initialized = false;
static bool pak_dirty = false;

// Flash storage address (must be sector-aligned)
#define FLASH_PAK_SECTOR (FLASH_STORAGE_OFFSET / FLASH_SECTOR_SIZE)

bool controller_pak_init(void) {
    // Initialize pak data to zeros
    memset(controller_pak_data, 0, CONTROLLER_PAK_SIZE);
    
    // Try to load existing data from flash
    controller_pak_load_from_flash();
    
    pak_initialized = true;
    pak_present = true;
    
    return true;
}

void controller_pak_read(uint16_t address, uint8_t* data, size_t length) {
    if (!pak_present || !pak_initialized) {
        // No pak present - return zeros
        memset(data, 0, length);
        return;
    }
    
    // Ensure address is within bounds
    if (address >= CONTROLLER_PAK_SIZE) {
        memset(data, 0, length);
        return;
    }
    
    // Limit length to available data
    if (address + length > CONTROLLER_PAK_SIZE) {
        length = CONTROLLER_PAK_SIZE - address;
    }
    
    // Copy data from pak memory
    memcpy(data, &controller_pak_data[address], length);
}

void controller_pak_write(uint16_t address, const uint8_t* data, size_t length) {
    if (!pak_present || !pak_initialized) {
        return;
    }
    
    // Ensure address is within bounds
    if (address >= CONTROLLER_PAK_SIZE) {
        return;
    }
    
    // Limit length to available space
    if (address + length > CONTROLLER_PAK_SIZE) {
        length = CONTROLLER_PAK_SIZE - address;
    }
    
    // Copy data to pak memory
    memcpy(&controller_pak_data[address], data, length);
    
    // Mark as dirty for later flash save
    pak_dirty = true;
    
    // Save to flash after a delay (to batch writes)
    // This would be better implemented with a timer
    static uint32_t last_write_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_write_time > 1000) { // 1 second delay
        controller_pak_save_to_flash();
        last_write_time = current_time;
    }
}

void controller_pak_format(void) {
    if (!pak_initialized) {
        return;
    }
    
    // Clear all data
    memset(controller_pak_data, 0, CONTROLLER_PAK_SIZE);
    
    // Set up basic controller pak structure
    // Note header area (first 256 bytes typically contain formatting info)
    // This is a simplified format - real N64 controller paks have a more complex structure
    
    // Write identification pattern
    controller_pak_data[0] = 0x81; // Controller pak ID
    controller_pak_data[1] = 0x80;
    controller_pak_data[2] = 0x80;
    controller_pak_data[3] = 0x80;
    
    pak_dirty = true;
    controller_pak_save_to_flash();
}

bool controller_pak_is_present(void) {
    return pak_present;
}

void controller_pak_save_to_flash(void) {
    if (!pak_dirty || !pak_initialized) {
        return;
    }
    
    // Disable interrupts during flash operation
    uint32_t interrupts = save_and_disable_interrupts();
    
    // Erase the flash sector
    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    
    // Write the controller pak data
    // Flash can only be written in 256-byte pages
    for (uint32_t offset = 0; offset < CONTROLLER_PAK_SIZE; offset += FLASH_PAGE_SIZE) {
        flash_range_program(FLASH_STORAGE_OFFSET + offset, 
                           &controller_pak_data[offset], 
                           FLASH_PAGE_SIZE);
    }
    
    // Restore interrupts
    restore_interrupts(interrupts);
    
    pak_dirty = false;
}

void controller_pak_load_from_flash(void) {
    // Read controller pak data from flash
    const uint8_t* flash_data = (const uint8_t*)(XIP_BASE + FLASH_STORAGE_OFFSET);
    
    // Check if flash contains valid data (not all 0xFF)
    bool flash_empty = true;
    for (int i = 0; i < 16; i++) {
        if (flash_data[i] != 0xFF) {
            flash_empty = false;
            break;
        }
    }
    
    if (!flash_empty) {
        // Copy data from flash to RAM
        memcpy(controller_pak_data, flash_data, CONTROLLER_PAK_SIZE);
    } else {
        // Flash is empty, format the pak
        controller_pak_format();
    }
} 