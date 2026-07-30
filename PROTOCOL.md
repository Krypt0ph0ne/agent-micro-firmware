# Agent Micro Raw HID protocol v2

The custom interface uses usage page `0xFF60`, usage `0x61`, and fixed 32-byte
input/output reports on endpoint 2. Keyboard/consumer reports use endpoint 1.

## Common packet format

| Byte | Meaning |
| ---: | --- |
| 0–1 | ASCII `CP` (`43 50`) |
| 2 | protocol version (`02`; firmware also accepts v1 configuration packets) |
| 3 | command/report type |
| 4–30 | command-specific payload, zero-filled when unused |
| 31 | XOR checksum of bytes 0–30 |

Visible control indices are `0…5` for physical K1…K6, `6` encoder-left, `7`
encoder-push and `8` encoder-right. The firmware translates the PCB's mirrored
key wiring internally.

## Host-to-device commands

| Command | Payload |
| --- | --- |
| `0x10` | Set LED: control, effect, R, G, B, period/20 ms, brightness in bytes 4…10 |
| `0x12` | Turn all LEDs off |
| `0x20` | Set binding: internal control, action kind, count, then five modifier/usage pairs |
| `0x30` | Request firmware status |
| `0x31` | Release all keyboard/consumer state and return status |

LED effects: `0` off, `1` steady, `2` blink, `3` pulse.

Binding kinds: `0` disabled, `1` keyboard tap/sequence, `2` consumer tap,
`3` app-only event, `4` held keyboard chord. Modifier bits are Ctrl `0x01`,
Shift `0x02`, Alt `0x04`, GUI/Command `0x08`; usages are USB HID key usages.

## Device-to-host reports

### Status (`0x80`)

| Byte | Meaning |
| ---: | --- |
| 4–6 | firmware major, minor, patch |
| 7 | capability flags |
| 8 | number of controls (`9`) |
| 9 | number of LEDs (`6`) |
| 10–11 | current visible pressed-control mask, little-endian |

Capability byte `0x1F` means press/release edges, Raw HID input events, held
bindings, status query and emergency release-all are supported.

### Physical event (`0x81`)

| Byte | Meaning |
| ---: | --- |
| 4 | wrapping sequence number |
| 5 | visible control index |
| 6 | phase: `0` release, `1` press, `2` encoder detent/trigger |
| 7–8 | current visible pressed-control mask, little-endian |

USB resets clear all internal keyboard and consumer reports without attempting
to transmit on an unconfigured bus. This prevents a held modifier/key from
remaining logically stuck after a disconnect.
