# BLE Protocol

This folder defines the shared BLE packet contract for:
- Control Node (CN)
- Sensor Node 1 (SN1)
- Sensor Node 2 (SN2)

[`ble_protocol.hpp`](ble_protocol.hpp) is the authoritative contract.

[`ble_protocol_example.cpp`](ble_protocol_example.cpp) is the sensor-node
peripheral example for SN1 or SN2 on hardware.

## Why The Packets Are Fixed

The packets are fixed-size and explicitly serialised so both sides agree on the
exact byte layout.

This avoids:
- order mismatches
- compiler issues
- floats on the wire
- bad unit conversions

All multi-byte values are little-endian as the hardware is naturally little-endian.

## Packet Summary

| Packet | Direction | BLE use | Size | Purpose |
| --- | --- | --- | ---: | --- |
| Telemetry | SN -> CN | Notify | 13 bytes | Periodic state snapshot / low-rate heartbeat |
| Event | SN -> CN | Notify | 8 bytes | Immediate critical transition |
| Control | CN -> SN | Write without response | 9 bytes | Commands, clears, actuator override |

## Wire Units

| Value | Type | Unit | Min | Max | Notes |
| --- | --- | --- | ---: | ---: | --- |
| temperature | `int16_t` | centi-degC | `-32768` | `32767` | `-327.68 C` to `327.67 C` |
| light | `uint16_t` | deci-lux | `0` | `65535` | `0.0 lux` to `6553.5 lux` |
| actuator duty | `uint8_t` | canonical duty | `0` | `255` | used for fan and LED |
| timestamp | `uint16_t` | ms modulo `65536` | `0` | `65535` | wraps every `65.535 s` |
| `status_word` | `uint16_t` | raw bits | `0` | `65535` | reserved in v1, normally `0` |
| `reserved` | `uint16_t` | raw bits | `0` | `65535` | must be `0` in v1 |

## Packet Layout

### Telemetry Packet

Size: `13` bytes

| Byte(s) | Field | Type | Meaning |
| --- | --- | --- | --- |
| 0 | `protocol_version` | `uint8_t` | Protocol version, currently `1` |
| 1 | `node_id` | `uint8_t` | Source node: `sn1` or `sn2` |
| 2 | `sequence` | `uint8_t` | Telemetry sequence number |
| 3-4 | `flags` | `uint16_t` | Persistent state flags |
| 5-6 | `temperature_centi_c` | `int16_t` | Temperature in centi-degC |
| 7-8 | `light_deci_lux` | `uint16_t` | Light in deci-lux |
| 9 | `fan_duty_applied` | `uint8_t` | Final fan duty currently used by the node |
| 10 | `led_duty_applied` | `uint8_t` | Final LED duty currently used by the node |
| 11-12 | `status_word` | `uint16_t` | Reserved snapshot word |

Telemetry flags:
- bit 0: `help_active` - help state is latched active
- bit 1: `local_actuator_control_selected` - local actuator control is currently selected
- bit 2: `sensor_fault` - sensor fault is latched active
- bit 3: `status_led_latched` - status LED latch is active
- bit 4: `button_pressed` - local button is currently pressed
- bit 5: `cn_actuator_control_selected` - CN actuator control is currently selected
- bit 6: `motion_detected` - motion detector output is currently active, used by SN1
- bit 7: `sound_detected` - sound detector output is currently active, used by SN2

### Event Packet

Size: `8` bytes

| Byte(s) | Field | Type | Meaning |
| --- | --- | --- | --- |
| 0 | `protocol_version` | `uint8_t` | Protocol version, currently `1` |
| 1 | `node_id` | `uint8_t` | Source node: `sn1` or `sn2` |
| 2 | `sequence` | `uint8_t` | Event sequence number |
| 3 | `event_type` | `uint8_t` | Immediate event type |
| 4-5 | `event_value` | `int16_t` | Event-specific value |
| 6-7 | `timestamp_ms_mod` | `uint16_t` | Timestamp modulo `65536 ms` |

Event types:
- `help_pressed`
- `help_released`
- `local_button_pressed`
- `local_button_released`
- `sensor_fault_asserted`
- `sensor_fault_cleared`
- `clear_acknowledged`
- `sound_detected` - asserted edge for the SN2 sound detector
- `motion_detected` - asserted edge for the SN1 motion detector

### Control Packet

Size: `9` bytes

| Byte(s) | Field | Type | Meaning |
| --- | --- | --- | --- |
| 0 | `protocol_version` | `uint8_t` | Protocol version, currently `1` |
| 1 | `target_node_id` | `uint8_t` | Target node: `sn1` or `sn2` |
| 2 | `command_sequence` | `uint8_t` | Command sequence number |
| 3-4 | `command_flags` | `uint16_t` | Control request flags |
| 5 | `fan_duty_override` | `uint8_t` | Fan override requested by CN |
| 6 | `led_duty_override` | `uint8_t` | LED override requested by CN |
| 7-8 | `reserved` | `uint16_t` | Must be `0` in v1 |

Control flags:
- bit 0: `actuator_override_enable`
- bit 1: `clear_help_request`
- bit 2: `clear_status_led_request`

## BLE Setup

Recommended setup:
- CN acts as the BLE central
- SN1 and SN2 act as BLE peripherals
- each sensor node exposes one custom service
- each sensor node exposes three characteristics:
  - telemetry: `NOTIFY`
  - event: `NOTIFY`
  - control: `WRITE_WO_RSP`

Why this setup:
- `NOTIFY` is a good fit for telemetry and urgent events sent from SN to CN
- `WRITE_WO_RSP` keeps control writes simple and low overhead
- one custom service keeps discovery straightforward
- longer connection intervals reduce radio activity and help lower power

The example in [`ble_protocol_example.cpp`](ble_protocol_example.cpp) shows:
- BLE bring-up with `BLE.on()` and `BLE.begin()`
- characteristic registration
- advertising the service UUID
- a preferred connection interval setup using `BLE.setPPCP()`
- periodic telemetry notify
- immediate event notify
- control write handling

It is a peripheral-side example. The CN would use the same protocol
pack/unpack helpers, but different BLE central logic.

The current example prefers a `100 ms` to `200 ms` connection interval. Some testing might be required to find the right balance.

## Communication Model

### Telemetry

Telemetry is the low-rate state snapshot.

Use it for:
- current temperature
- current light
- current fan duty
- current LED duty
- current motion detector state
- current sound detector state
- current persistent state flags

Telemetry should report the final values currently being used by the node. For
fan and LED, that means the applied output after any local/remote selection.

Recommended behaviour:
- send periodically at the default `5000 ms` interval
- also send on meaningful delta
- newer telemetry supersedes older telemetry
- no explicit ACK required

In practice, telemetry is the the low-rate heartbeat.

### Event

Events are for urgent transitions.

Use them for:
- help pressed / released
- local button pressed / released
- fault asserted / cleared
- sound detector asserted
- motion detector asserted

Recommended behaviour:
- send immediately
- use the event sequence number
- reject duplicates or stale packets with `ble_event_should_accept()`
- keep critical state latched in telemetry flags as well

That last point matters. If one event is missed, the next telemetry packet still
shows the current state. For motion and sound, the event is the asserted edge (i.e., the interrupt)
and the telemetry flag carries the current binary level.

### Control

Controls are commands from CN to a specific sensor node.

Use them for:
- actuator override updates
- clear help requests
- clear status LED requests

The control packet carries the override requested by CN. It does not prove that
the node is still using that value later. The confirmation path is the next
telemetry snapshot showing the applied output and current flags.

Recommended behaviour:
- include a `command_sequence`
- reject duplicates or stale commands with `ble_command_should_accept()`
- apply each accepted command once only

Suggested peripheral behaviour:
- start in local mode
- keep a local duty request and a remote override request separately
- if remote override is enabled, apply the remote value
- if remote override is cleared, return to the local value
- telemetry should report the final applied output, not just the last remote request, i.e., what the node is doing at that moment.

## ACK / Confirmation

There is no separate dedicated ACK packet type in protocol v1.

- SN -> CN critical events:
  - no explicit ACK packet
  - reliability comes from sequence numbers and persistent telemetry flags
- CN -> SN control commands:
  - no mandatory dedicated ACK packet
  - main confirmation is the next telemetry state echo
  - `clear_acknowledged` may be sent after a clear is applied, but telemetry is
    still the main confirmation path

If stronger positive acknowledgement is needed for every critical message, it is doable but that would be a complete protocol change.

This is a compromise between low power consumption but still having a robust link. Instead of a dedicated ACK packet, we "piggyback" the telemetry packet with the confirmation.