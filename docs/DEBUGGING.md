# Debugging Guide

This guide provides troubleshooting steps and debugging techniques for the RP2040 N64 Controller Emulator.

## Status LED Indicators

The onboard LED provides visual feedback about the system state:

### Startup Sequence
- **1 long blink (200ms)**: Power-on indicator
- **2 short blinks (100ms each)**: Successful initialization
- **Slow pulse (1Hz)**: Normal operation heartbeat

### Error Indicators
- **5 fast blinks**: Controller pak initialization failed
- **10 fast blinks**: N64 protocol initialization failed
- **Continuous fast blinking**: System initialization failed
- **3 quick blinks**: Reset condition detected (L+R+Start pressed)

## Serial Debug Output

If `DEBUG_ENABLE` is set to 1 in `config.h`, the system outputs debug information via USB serial.

### Connecting to Serial Output
```bash
# Linux/macOS
screen /dev/ttyACM0 115200

# Windows (replace COM3 with actual port)
putty -serial COM3 -sercfg 115200,8,n,1,N

# Alternative: Use Arduino IDE Serial Monitor
```

### Debug Output Format
```
RP2040 N64 Controller Emulator Starting...
Encoder system initialized
Button system initialized
Controller pak initialized
N64 protocol initialized
System initialization complete
Waiting for N64 console commands...
Stick: X=0, Y=0, Buttons=0x0000
Stick: X=12, Y=-8, Buttons=0x0001
```

## Common Issues and Solutions

### 1. Controller Not Detected by N64

**Symptoms:**
- N64 console shows "No Controller" message
- Controller test shows no response

**Possible Causes & Solutions:**

#### Power Issues
- **Check 3.3V supply**: Measure voltage between 3V3 and GND pins
- **Verify current capacity**: Ensure power supply can provide at least 100mA
- **Check for shorts**: Use multimeter to verify no shorts between power pins

#### Data Line Issues
- **Verify data pin connection**: Check GP0 to N64 data line continuity
- **Check signal integrity**: Use oscilloscope to verify data line idles at 3.3V
- **Test with known good cable**: Replace N64 extension cable if suspect

#### Timing Issues
- **Crystal accuracy**: Some RP2040 boards have inaccurate crystals
- **PIO clock settings**: May need to adjust clock divider in PIO program
- **Interrupt latency**: Check if other processes are interfering

### 2. Analog Stick Not Working

**Symptoms:**
- Stick position always reads as center (0,0)
- Stick movement doesn't register
- Erratic or jittery movement

**Debugging Steps:**

#### Check Encoder Power
```bash
# With multimeter, check:
# - 3.3V between VCC and GND on stick module
# - Continuity from RP2040 3V3 pin to stick VCC
# - Continuity from RP2040 GND to stick GND
```

#### Verify Encoder Signals
```bash
# Enable debug output and move stick
# Should see position changes in serial output:
# Stick: X=12, Y=-8, Buttons=0x0000
```

#### Test Encoder Connectivity
```c
// Add this debug code to main.c
void debug_encoder_pins(void) {
    printf("Encoder pins: X1=%d X0=%d Y1=%d Y0=%d\n",
           gpio_get(ENCODER_X1_PIN),
           gpio_get(ENCODER_X0_PIN),
           gpio_get(ENCODER_Y1_PIN),
           gpio_get(ENCODER_Y0_PIN));
}
```

#### Check Quadrature Signals
- Use oscilloscope to monitor encoder pins while moving stick
- Should see square wave patterns with 90° phase shift between A and B signals
- Frequency should change with movement speed

### 3. Buttons Not Working

**Symptoms:**
- Button presses not registered
- Wrong buttons activated
- Buttons stuck "on"

**Debugging Steps:**

#### Check Button Wiring
```c
// Add button debug code
void debug_buttons(void) {
    for (int i = 0; i < 14; i++) {
        printf("Button %d: %d\n", i, !gpio_get(button_pins[i]));
    }
}
```

#### Verify Pull-up Resistors
- Buttons should read HIGH when not pressed
- Should read LOW when pressed
- If inverted, check wiring or modify button reading logic

### 4. Communication Protocol Issues

**Symptoms:**
- Controller detected but doesn't respond correctly
- Intermittent operation
- Console freezes when controller connected

**Debugging Protocol Communication:**

#### Enable Protocol Debug
```c
// Add to n64_protocol.c
void debug_command(uint8_t command) {
    printf("Received N64 command: 0x%02X\n", command);
}
```

#### Check Timing with Oscilloscope
- Data line should idle at 3.3V
- Logic 0: 3μs low, 1μs high
- Logic 1: 1μs low, 3μs high
- Verify stop bit timing

#### Monitor Command Sequence
```
Expected sequence:
1. Console sends 0x00 (info request)
2. Controller responds: 0x05 0x00 0x02
3. Console sends 0x01 (poll request)
4. Controller responds: button data + stick data
```

## Advanced Debugging Techniques

### Logic Analyzer Setup

For precise timing analysis:

1. **Connect Logic Analyzer**:
   - Channel 0: N64 data line
   - Channel 1: RP2040 status indicator
   - Channel 2-5: Encoder signals (X1, X0, Y1, Y0)

2. **Trigger Settings**:
   - Trigger on falling edge of data line
   - Capture at least 1ms of data
   - Sample rate: 10MHz minimum

3. **Analysis**:
   - Verify bit timing matches N64 specification
   - Check for glitches or noise
   - Measure response delay times

### Flash Memory Debugging

To debug controller pak issues:

```c
void debug_flash_content(void) {
    const uint8_t* flash_data = (const uint8_t*)(XIP_BASE + FLASH_STORAGE_OFFSET);
    printf("Flash content (first 16 bytes):\n");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", flash_data[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
}
```

### Performance Monitoring

Monitor system performance:

```c
void debug_timing(void) {
    static uint32_t last_time = 0;
    uint32_t current_time = time_us_32();
    uint32_t delta = current_time - last_time;
    
    if (delta > 2000) { // More than 2ms between updates
        printf("Warning: Long update interval: %u us\n", delta);
    }
    
    last_time = current_time;
}
```

## Oscilloscope Patterns

### Normal Operation Patterns

#### N64 Data Line (Console Poll Command)
```
     ___     ___     ___     ___
____|   |___|   |___|   |___|   |___
    3μs 1μs 1μs 3μs 3μs 1μs 1μs 3μs
    0   0   1   1   0   0   0   1
    (Command 0x01)
```

#### Encoder Quadrature Signals
```
X1: ___   ___   ___   ___
      |___|   |___|   |___|
      
X0:  ___   ___   ___   ___
    |___|   |___|   |___|
    
(90° phase shift for direction detection)
```

### Problem Patterns

#### Incorrect Timing
- Bit periods too long/short
- Uneven high/low times
- Missing stop bits

#### Noisy Encoder Signals
- Multiple transitions per step
- Irregular pulse widths
- DC offset issues

## Test Procedures

### Basic Functionality Test

1. **Power-on Test**:
   - Connect RP2040 via USB
   - Verify LED startup sequence
   - Check serial output for initialization messages

2. **Encoder Test**:
   - Move stick in all directions
   - Verify position changes in debug output
   - Test center position accuracy

3. **Button Test**:
   - Press each button individually
   - Verify correct bit patterns in debug output
   - Test button combinations

4. **N64 Console Test**:
   - Connect to N64 console
   - Power on console
   - Check controller detection
   - Test in simple game (Mario 64)

### Comprehensive Test

1. **Protocol Compliance**:
   - Test all N64 commands (0x00, 0x01, 0x02, 0x03, 0xFF)
   - Verify timing specifications
   - Test controller pak operations

2. **Stress Test**:
   - Rapid stick movements
   - Multiple button combinations
   - Extended operation periods

3. **Compatibility Test**:
   - Test with multiple N64 consoles
   - Test with various games
   - Test with different cable lengths

## Getting Help

If you're still experiencing issues:

1. **Check hardware connections** against the wiring diagrams
2. **Enable debug output** and capture serial logs
3. **Use oscilloscope** to verify signal timing if available
4. **Try with minimal configuration** (encoder only, no buttons)
5. **Test with known-good N64 controller** for comparison

### Reporting Issues

When reporting issues, please include:
- Complete debug serial output
- Description of hardware setup
- Oscilloscope captures (if available)
- Steps to reproduce the problem
- N64 console model and game tested 