#ifndef CONTROLLER_PAK_H
#define CONTROLLER_PAK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Function prototypes
bool controller_pak_init(void);
void controller_pak_read(uint16_t address, uint8_t* data, size_t length);
void controller_pak_write(uint16_t address, const uint8_t* data, size_t length);
void controller_pak_format(void);
bool controller_pak_is_present(void);

// Internal functions
void controller_pak_save_to_flash(void);
void controller_pak_load_from_flash(void);

#endif // CONTROLLER_PAK_H 