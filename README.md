# ESP32 Robotic Arm

Manual control of four actuators—shoulder, elbow, hand, and claw—using potentiometers, an ESP32, and SG90 servos.

**Status:** Prototype under development. Initial firmware provided by the project author.

## Features

- Reads one potentiometer per actuator.
- Averages 10 ADC readings and applies an exponential filter.
- Converts the filtered reading into an angle command.
- Updates each servo when the difference from its previous command is at least 2°.
- Displays commanded angles in the serial monitor at 115200 baud.

The displayed values represent commanded angles. The project does not measure actual joint positions, implement inverse kinematics, or record movement sequences.

## Hardware

| Component | Quantity | Specifications / Notes |
| --- | --- | --- |
| ESP32 | 1 | ESP32 |
| Servo | 4 | SG90 |
| Potentiometer | 4 | 10 kΩ |
| Power supply | 1 | 5 V; reported current rating: 1 A ± 0.5 A |
| Capacitor | 8 | Ceramic 100 µF |
| Capacitor | 1 | Electrolytic 1000 nF |
| Resistor | 4 | 1 kΩ |

Servo and potentiometer quantities correspond to the four channels defined in the firmware. The physical assembly still needs to be verified on the bench.

## Repository Structure

```text
esp32-robotic-arm/
├── README.md
├── firmware/
│   └── robotic-arm/
│       └── robotic-arm.ino
├── hardware/
│   ├── README.md
|   └──components
|       └──(imgs.png)
|
└── media/
```

## Getting Started

1. Review the [wiring,components, schematic and  power supply documentation](Hardware/README.md).
2. Understand the [firmware](firmware/robotic-arm/README.md).


The `Joint` class is explained in the [firmware operation guide](docs/funcionamento.md).

## Known Limitations

The firmware preserves the original control logic. Calibration parameters are not yet validated in code: equal ADC minimum and maximum values cause division by zero.

The 0–180° angle limits and 500–2400 µs pulse widths are firmware settings, not a verified mechanical operating range for the installed SG90 servos.

During initialization, each servo receives a command based on its potentiometers current position, which may cause immediate movement.

## Roadmap

- [ ] Read and understand the [schematic](Hardware/README.md).
- [ ] Make the connections.
- [ ] Install ARDUINO IDE and **ESP32Servo** Library.
- [ ] Configure the Arduino to ESP32 Board
- [ ] Understand the [firmware](firmware/robotic-arm/README.md)
- [ ] Input the [code](firmware/robotic-arm/robotic-arm.ino) and set max and min values.
- [ ] Validate configuration parameters and handle servo attachment failures in the firmware.
- [ ] Be Happy :)

## Next steps

- [ ] Create one online server to control the robotic arm in the internet.
- [ ] Automatize tasks with pre configured movements.
