#ifndef CONFIG_H
#define CONFIG_H

// N64 Protocol Pin Configuration
#define N64_DATA_PIN 0

// N64 Stick Encoder Pin Configuration
#define ENCODER_Y1_PIN 2  // Y-axis encoder A
#define ENCODER_Y0_PIN 3  // Y-axis encoder B
#define ENCODER_X1_PIN 4  // X-axis encoder A
#define ENCODER_X0_PIN 5  // X-axis encoder B

// Button Pin Configuration (optional - can be connected to other devices)
#define BUTTON_A_PIN     6
#define BUTTON_B_PIN     7
#define BUTTON_Z_PIN     8
#define BUTTON_START_PIN 9
#define BUTTON_DU_PIN    10
#define BUTTON_DD_PIN    11
#define BUTTON_DL_PIN    12
#define BUTTON_DR_PIN    13
#define BUTTON_L_PIN     14
#define BUTTON_R_PIN     15
#define BUTTON_CU_PIN    16
#define BUTTON_CD_PIN    17
#define BUTTON_CL_PIN    18
#define BUTTON_CR_PIN    19

// Status LED
#define STATUS_LED_PIN 25

// N64 Controller Configuration
#define N64_CONTROLLER_ID_HIGH 0x05
#define N64_CONTROLLER_ID_LOW  0x00

// Stick Configuration
#define STICK_DEADZONE 2
#define STICK_MAX_VALUE 127
#define STICK_MIN_VALUE -128
#define ENCODER_SCALE_FACTOR 4  // Each encoder step represents 4 controller units

// Timing Configuration (in microseconds)
#define N64_BIT_PERIOD_US 4
#define N64_LOGIC_0_LOW_US 3
#define N64_LOGIC_0_HIGH_US 1
#define N64_LOGIC_1_LOW_US 1
#define N64_LOGIC_1_HIGH_US 3
#define N64_STOP_CONSOLE_LOW_US 1
#define N64_STOP_CONSOLE_HIGH_US 2
#define N64_STOP_CONTROLLER_LOW_US 2
#define N64_STOP_CONTROLLER_HIGH_US 1

// PIO Configuration
#define N64_PIO pio0
#define N64_PIO_SM 0

// Debug Configuration
#define DEBUG_ENABLE 1
#define DEBUG_UART_BAUD 115200

// Controller Pak Configuration
#define CONTROLLER_PAK_SIZE 32768  // 32KB
#define CONTROLLER_PAK_PAGE_SIZE 32
#define CONTROLLER_PAK_PAGES (CONTROLLER_PAK_SIZE / CONTROLLER_PAK_PAGE_SIZE)

// Flash Storage Configuration
#define FLASH_STORAGE_OFFSET (1024 * 1024)  // 1MB offset from start of flash

#endif // CONFIG_H 