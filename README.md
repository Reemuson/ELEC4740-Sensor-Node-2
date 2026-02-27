# Sensor Node 2 (SN2)

Firmware for **Sensor Node 2 (SN2)** in the ELEC4740 Internet of Things
design project. SN2 is responsible for **environmental monitoring**
and **local actuation**, and operates as a **Bluetooth Low Energy (BLE)
peripheral**.

---

## Project Context

- Course: ELEC4740 – Internet of Things
- Platform: Particle Photon 2
- Device OS: 6.3.4
- Language: C++
- Role: BLE Peripheral (sensor node)

SN2 communicates with other nodes using a shared BLE protocol defined
in a separate protocol module.

---

## Responsibilities

### Part 1 (Standalone Operation)

In Part 1, SN2 is responsible for:

- Temperature sensing and calibration
- Sound detection and thresholding
- PWM control of a fan actuator
- Local user interface:
  - Help button
  - Status LEDs
- Periodic telemetry generation
- Event detection and reporting (local)

### Part 2 (System Integration)

In Part 2, SN2 will additionally:

- Communicate with a BLE control node
- Transmit telemetry and events over BLE
- Accept override commands for fan control
- Support integrated system demonstrations

---

## BLE Protocol

All BLE communication conforms to the shared protocol contract defined in:

`protocol/ble_protocol.hpp`

This file is **authoritative** and defines:

- UUIDs
- Packet layouts and sizes
- Scaling rules
- Event semantics
- Control behaviour

SN2 must not deviate from this definition.

---

## Repository Structure

```
├── src/ # Application source code
├── lib/ # Local libraries and drivers
├── protocol/ # Shared BLE protocol definition
│ ├── ble_protocol.hpp
│ └── README.md
├── project.properties # Particle project configuration
└── README.md # This file
```

---

## Build and Flash

This project is built using Particle Developer Tools.

Typical workflow:
1. Open the project folder in VS Code
2. Select the target device (Photon 2)
3. Compile and flash via USB
4. Monitor logs using the Particle Serial Monitor

> Detailed build instructions will be added once firmware features stabilise.

---

## Design Notes

- All BLE payloads use fixed-width integers
- No floating-point values are transmitted over BLE
- No dynamic memory allocation is used
- Protocol structures are compile-time size checked
- Behaviour is deterministic and event-driven

These design choices support reliability, testability and predictable
integration.

---

## Status

- [ ] Hardware bring-up
- [ ] Sensor drivers
- [ ] Local UI
- [ ] PWM actuator control
- [ ] BLE advertising and characteristics
- [ ] Telemetry transmission
- [ ] Event transmission
- [ ] Control override handling

---

## Notes and TODO

- Final sensor selection and calibration details to be documented
- Power consumption measurements to be added in Part 2
- Integration notes with control node to be added post-integration

---

## Calibration of NTC

```
cal start
cal set r_series_ohms 100000
cal set ref_mv 3300

cal point set 1 500
cal point capture 1

cal point set 2 2500
cal point capture 2

cal point set 3 4500
cal point capture 3

cal solve
cal save
cal show
```

## Authors

- Ryan Hicks

---

## License

MIT