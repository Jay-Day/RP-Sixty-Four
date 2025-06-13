# Hardware Setup Guide

This guide provides detailed instructions for setting up the hardware for the RP2040 N64 Controller Emulator.

## Required Components

### Core Components
- **Raspberry Pi Pico** (or compatible RP2040 board)
- **Nintendo 64 Analog Stick Module** (with wheel encoders)
- **N64 Controller Extension Cable** (or salvaged controller connector)

### Optional Components
- **Buttons/Switches** (for physical button inputs)
- **Pull-up Resistors** (4.7kΩ - usually not needed with internal pull-ups)
- **Level Shifter** (if interfacing with 5V logic - usually not needed)
- **Breadboard/PCB** for prototyping

## N64 Stick Module Pinout

The N64 analog stick module uses a 6-pin JST connector:

```
Pin 1: Y1 (Y-axis encoder signal A)
Pin 2: Y0 (Y-axis encoder signal B)  
Pin 3: X1 (X-axis encoder signal A)
Pin 4: GND (Ground)
Pin 5: VCC (3.3V Power)
Pin 6: X0 (X-axis encoder signal B)
```

### Identifying the Connector

The connector is typically a white 6-pin JST connector. Pin 1 is usually marked with a small triangle or different colored wire (often white).

## Wiring Connections

### RP2040 to N64 Stick Module

| RP2040 Pin | N64 Stick Pin | Wire Color (typical) | Function |
|------------|---------------|---------------------|----------|
| GP2        | Pin 1 (Y1)    | White              | Y-axis encoder A |
| GP3        | Pin 2 (Y0)    | Yellow             | Y-axis encoder B |
| GP4        | Pin 3 (X1)    | Green              | X-axis encoder A |
| GND        | Pin 4 (GND)   | Black              | Ground |
| 3V3        | Pin 5 (VCC)   | Red                | 3.3V Power |
| GP5        | Pin 6 (X0)    | Blue               | X-axis encoder B |

### RP2040 to N64 Console

The N64 controller port has 3 connections:

| RP2040 Pin | N64 Port | Function |
|------------|----------|----------|
| GP0        | Data     | Bi-directional data line |
| 3V3        | VCC      | 3.3V Power |
| GND        | GND      | Ground |

### Optional Button Connections

If you want to add physical buttons:

| RP2040 Pin | Button Function |
|------------|----------------|
| GP6        | A Button |
| GP7        | B Button |
| GP8        | Z Button (Trigger) |
| GP9        | Start Button |
| GP10       | D-pad Up |
| GP11       | D-pad Down |
| GP12       | D-pad Left |
| GP13       | D-pad Right |
| GP14       | L Trigger |
| GP15       | R Trigger |
| GP16       | C-Up |
| GP17       | C-Down |
| GP18       | C-Left |
| GP19       | C-Right |

Connect buttons between the GPIO pin and GND. The firmware uses internal pull-ups.

## Assembly Instructions

### Step 1: Prepare the N64 Stick Module

1. **Obtain an N64 stick module** - You can salvage one from a broken N64 controller or purchase a replacement online
2. **Check the connector** - Ensure the 6-pin connector is intact
3. **Test continuity** - Use a multimeter to verify connections from the connector to the encoder boards

### Step 2: Prepare the RP2040 Board

1. **Solder headers** if not already attached
2. **Test basic functionality** by uploading a simple blink program

### Step 3: Make Connections

1. **Connect the stick module** to the RP2040 according to the wiring table
2. **Connect the N64 data line** (this requires careful handling of the N64 controller connector)
3. **Add optional buttons** if desired
4. **Double-check all connections** with a multimeter

### Step 4: N64 Controller Port Connection

#### Option 1: Extension Cable (Recommended)
- Purchase an N64 controller extension cable
- Cut the cable and identify the three wires
- Connect to the RP2040 as shown in the wiring table

#### Option 2: Direct Controller Modification
- **WARNING:** This will permanently modify an N64 controller
- Remove the controller PCB
- Identify the three controller port connections
- Solder wires directly to the connector

#### Option 3: Build Custom Connector
- The N64 connector is a proprietary 3-pin design
- 3D printed adapter may be possible with careful measurements
- Ensure proper pin spacing and contact reliability

## Testing and Verification

### Power-On Test
1. Connect power to the RP2040
2. The status LED should blink once on power-up
3. After initialization, it should blink slowly (heartbeat)

### Encoder Test
1. Move the analog stick
2. Monitor serial output (if debug enabled) to see position changes
3. Verify X and Y axes respond correctly

### N64 Console Test
1. Connect to an N64 console
2. Power on the console
3. The controller should be detected
4. Test in a game that uses analog stick input

## Troubleshooting

### Controller Not Detected
- **Check power connections** - Verify 3.3V and GND are properly connected
- **Verify data line** - Ensure the data pin connection is solid
- **Check for shorts** - Use multimeter to verify no shorts between pins

### Stick Not Working
- **Verify encoder power** - Check that the stick module is receiving 3.3V
- **Test encoder signals** - Use an oscilloscope to verify quadrature signals when moving stick
- **Check pin assignments** - Ensure X and Y encoder pins are connected correctly

### Intermittent Operation
- **Loose connections** - Check all solder joints and wire connections
- **Power supply issues** - Ensure adequate power supply for the RP2040
- **Electromagnetic interference** - Keep wires short and use twisted pairs if possible

## Safety Considerations

- **Never connect while console is powered** - Always power off the N64 before connecting/disconnecting
- **Use proper voltage levels** - N64 uses 3.3V logic, which matches the RP2040
- **Avoid static discharge** - Use proper ESD precautions when handling components
- **Double-check polarity** - Incorrect power connections can damage components

## Performance Optimization

### Signal Integrity
- Keep encoder wires as short as practical
- Use twisted pair cables for encoder signals if available
- Add small capacitors (10-100nF) near the encoder power pins if noise is an issue

### Mechanical Considerations
- Secure all connections to prevent movement during use
- Consider strain relief for the N64 cable connection
- Ensure the stick module is properly mounted to prevent mechanical stress

## Advanced Modifications

### Multiple Controller Support
- The RP2040 has enough pins to support multiple controller emulation
- Would require additional PIO state machines and careful timing coordination

### Custom PCB Design
- Consider designing a custom PCB for a more professional installation
- Include test points for debugging
- Add status LEDs for system state indication

### 3D Printed Enclosure
- Design an enclosure that accommodates the RP2040 and stick module
- Include mounting points for buttons if desired
- Ensure adequate ventilation and access to programming ports 