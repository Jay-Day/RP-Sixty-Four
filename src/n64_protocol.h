#ifndef N64_PROTOCOL_H
#define N64_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// N64 Commands
#define N64_CMD_INFO    0x00
#define N64_CMD_POLL    0x01
#define N64_CMD_READ    0x02
#define N64_CMD_WRITE   0x03
#define N64_CMD_RESET   0xFF

// Controller response structure for poll command
typedef struct {
    uint16_t buttons;    // Button state (14 bits used)
    int8_t stick_x;      // X-axis position (-128 to 127)
    int8_t stick_y;      // Y-axis position (-128 to 127)
} n64_controller_state_t;

// Controller info response
typedef struct {
    uint8_t id_high;     // Controller ID high byte (0x05)
    uint8_t id_low;      // Controller ID low byte (0x00)
    uint8_t status;      // Status byte
} n64_controller_info_t;

// Function prototypes
bool n64_protocol_init(void);
void n64_protocol_task(void);
void n64_protocol_update_state(const n64_controller_state_t* state);
void n64_protocol_reset(void);

// Internal functions
void n64_send_byte(uint8_t data);
void n64_send_stop_bit(void);
uint8_t n64_receive_byte(void);
void n64_handle_command(uint8_t command);

// Status bits
#define N64_STATUS_CRC_ERROR      0x04
#define N64_STATUS_PAK_REMOVED    0x02
#define N64_STATUS_PAK_INSERTED   0x01

#endif // N64_PROTOCOL_H 