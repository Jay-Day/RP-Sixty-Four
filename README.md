# RP2040 N64 Controller Emulator

An RP2040-based Nintendo 64 controller emulator that reads wheel encoders from the N64 controller stick module and emulates OEM controller behavior.

## Features

- **Complete N64 Joybus Protocol Implementation**: Emulates all standard N64 controller commands
- **Wheel Encoder Support**: Reads quadrature signals from original N64 stick modules
- **High-Precision Timing**: Uses RP2040 PIO for microsecond-accurate protocol timing
- **OEM-Compatible**: Passes all console compatibility checks
- **Low Latency**: Sub-millisecond response times
- **Configurable**: Easy to modify button mappings and stick sensitivity

## Hardware Requirements

### RP2040 Board
- Raspberry Pi Pico or compatible RP2040 board
- Must run at 3.3V (N64 uses 3.3V logic levels)

### N64 Stick Module
- Original Nintendo 64 analog stick module with wheel encoders
- 6-pin JST connector (or equivalent wiring)
![image](https://github.com/user-attachments/assets/ad451662-a90e-4fe0-9e4c-e4d87ddb7662)


### N64 Controller Connection
- 3-pin N64 controller connector
- Level shifter if using 5V logic (optional)

## Pin Connections

### N64 Console Connection
```
RP2040 Pin  | N64 Controller Port
GP0         | Data Line
3V3         | VCC (3.3V)
GND         | Ground
```

### N64 Stick Module Connection
```
RP2040 Pin  | N64 Stick Pin | Function
GP2         | Pin 1 (Y1)    | Y-axis encoder A
GP3         | Pin 2 (Y0)    | Y-axis encoder B  
GP4         | Pin 3 (X1)    | X-axis encoder A
GND         | Pin 4 (GND)   | Ground
3V3         | Pin 5 (VCC)   | 3.3V Power
GP5         | Pin 6 (X0)    | X-axis encoder B
```

### Button Connections (optional)
```
RP2040 Pin  | Button
GP6         | A Button
GP7         | B Button
GP8         | Z Button (Trigger)
GP9         | Start Button
GP10        | D-Up
GP11        | D-Down
GP12        | D-Left
GP13        | D-Right
GP14        | L Trigger
GP15        | R Trigger
GP16        | C-Up
GP17        | C-Down
GP18        | C-Left
GP19        | C-Right
```

## N64 Protocol Overview

The N64 uses a custom 1-wire half-duplex protocol:
- **Baud Rate**: 250 kHz
- **Logic 0**: 3μs low, 1μs high
- **Logic 1**: 1μs low, 3μs high
- **Stop Bit**: Console = 1μs low, 2μs high; Controller = 2μs low, 1μs high

### Supported Commands
- `0x00`: Controller Info/Status
- `0x01`: Poll Controller State
- `0x02`: Read Controller Pak
- `0x03`: Write Controller Pak
- `0xFF`: Reset Controller

## Building and Installation

### Prerequisites
- Raspberry Pi Pico SDK
- CMake 3.13+
- GCC ARM toolchain

### Build Steps
```bash
git clone https://github.com/your-username/rp2040-n64-controller
cd rp2040-n64-controller
mkdir build
cd build
cmake ..
make
```

### Installation
1. Hold BOOTSEL button on Pico
2. Connect via USB
3. Copy `n64_controller.uf2` to the mounted drive

## Configuration

Edit `config.h` to customize:
- Button pin assignments
- Stick sensitivity and calibration
- Protocol timing adjustments
- Debug output settings

## Technical Details

### Wheel Encoder Reading
The N64 stick uses quadrature encoders with:
- 80 slits per encoder disk
- 60-tooth gear arm driving 18-tooth encoder gear (10:3 ratio)
- 1 encoder step = 1.35° joystick deflection
- ±24.3° total travel range
- Linear relationship between angle and encoder pulses

### Timing Implementation
Uses RP2040 PIO state machines for precise timing:
- PIO handles protocol bit timing
- Interrupt-driven encoder reading
- Main core handles button scanning and protocol logic

### Memory Usage
- ~32KB flash for firmware
- ~4KB RAM for runtime data
- Remainder available for controller pak emulation

## Troubleshooting

### Controller Not Detected
- Check 3.3V power supply
- Verify data line connection
- Ensure proper grounding

### Stick Not Working
- Verify encoder connections
- Check encoder power (3.3V)
- Test with multimeter for continuity

### Timing Issues
- May need to adjust PIO clock divider
- Check crystal accuracy on RP2040

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Nintendo for the N64 hardware specifications
- [qwertymodo](https://www.qwertymodo.com/hardware-projects/n64/n64-controller) for detailed protocol analysis
- [n64brew](https://n64brew.dev/wiki/Joybus_Protocol) community for protocol documentation
- RP2040 community for PIO examples and documentation

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 
