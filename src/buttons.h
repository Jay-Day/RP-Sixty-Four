#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

// Button bit masks for N64 controller format
#define N64_BUTTON_A     (1 << 0)
#define N64_BUTTON_B     (1 << 1)
#define N64_BUTTON_Z     (1 << 2)
#define N64_BUTTON_START (1 << 3)
#define N64_BUTTON_DU    (1 << 4)
#define N64_BUTTON_DD    (1 << 5)
#define N64_BUTTON_DL    (1 << 6)
#define N64_BUTTON_DR    (1 << 7)
#define N64_BUTTON_L     (1 << 8)
#define N64_BUTTON_R     (1 << 9)
#define N64_BUTTON_CU    (1 << 10)
#define N64_BUTTON_CD    (1 << 11)
#define N64_BUTTON_CL    (1 << 12)
#define N64_BUTTON_CR    (1 << 13)

// Reset condition (L + R + Start)
#define N64_RESET_MASK (N64_BUTTON_L | N64_BUTTON_R | N64_BUTTON_START)

// Function prototypes
void buttons_init(void);
uint16_t buttons_read(void);
bool buttons_is_reset_pressed(void);

#endif // BUTTONS_H 