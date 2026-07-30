# Agent Micro firmware

Open firmware for the tested SinLoon SL2024502 six-key, one-encoder CH552
macropad sold under ASIN `B0DN9T9J75`.

> [!WARNING]
> Flashing replaces the factory application firmware. It could not be backed
> up, so a complete return to the seller's original state is not guaranteed.
> Visually identical revisions can use different wiring. Confirm the model,
> USB identities, and SW2 location before continuing.

Agent Micro is an independent community project. It is not an official
SinLoon, Amazon, OpenAI, or Anthropic product. This repository licenses only
the firmware and software—not the commercial PCB, enclosure, or hardware.

## Current release status

The source is ready for experimental builds. The current custom identity
`4249:4287` is a legacy, locally selected ID and is not officially allocated.
A free pid.codes identity under VID `0x1209` is being requested. No stable
firmware binary will be published before that assignment is accepted.

| Mode | VID:PID |
| --- | --- |
| Factory application | `1189:8890` |
| WCH ROM bootloader | `4348:55e0` |
| Legacy experimental firmware | `4249:4287` |
| Stable Agent Micro firmware | pending pid.codes |

The companion source-only macOS app is
[agent-micro](https://github.com/Krypt0ph0ne/agent-micro).

## Supported hardware

The tested revision has USB-C at the top, six keys, one rotary encoder, six
addressable RGB LEDs, and a CH552G. Its verified pin and LED maps are in
[HARDWARE_NOTES.md](HARDWARE_NOTES.md). Do not infer compatibility from the
Amazon listing or enclosure alone.

Working features include all six keys, encoder left/right/push, keyboard and
consumer HID actions, true held-key reports, six RGB LEDs, Raw HID
configuration/events, status, and emergency release-all. The fixed 32-byte
wire format remains Raw HID protocol v2 with `CP` magic for compatibility; see
[PROTOCOL.md](PROTOCOL.md).

## Build

The reproducible toolchain is SDCC 4.6.0. On macOS:

```bash
xcode-select --install
brew install sdcc

git clone https://github.com/Krypt0ph0ne/agent-micro-firmware.git
cd agent-micro-firmware

make doctor
make clean all
make tools
make test
```

Everything generated is placed below `build/`:

- `build/agent-micro-firmware.bin`
- `build/agent-micro-firmware.hex`
- `build/tools/…` native tools built from the checked-in C++ sources

`make doctor` rejects an SDCC version other than 4.6.0. `make test` performs a
clean build, enforces the `0x3800` application limit, builds the native tools,
checks shell syntax, and prints SHA-256 hashes.

## Flashing and recovery

Read the complete macOS procedure before bridging or flashing anything:
[docs/flashing-macos.md](docs/flashing-macos.md).

The short command sequence is:

```bash
shasum -a 256 build/agent-micro-firmware.bin
sudo ./tools/run_native_preflight_sequence.sh
sudo ./tools/run_native_flash_sequence.sh

ioreg -p IOUSB -l -w 0 |
  grep -E 'Agent Micro|"idVendor"|"idProduct"'
```

The preflight path performs no flash write. The flash path requires the
explicit `--confirm-replace-factory` argument internally and does not contain
DataFlash, configuration-word, bootloader, OTP, or protection-removal
operations. Both wrappers execute only tools compiled locally by `make tools`.

## Contributing and security

See [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. Sign each
commit with `git commit -s`. Report vulnerabilities privately as described in
[SECURITY.md](SECURITY.md); do not include device UIDs, DataFlash dumps, or
other device-specific backups in issues.

## License and attribution

The firmware is licensed under CC BY-SA 3.0. The native tools and `chprog.py`
are separately MIT licensed. Attribution, changes, and the CH554 header's
provenance are recorded in [NOTICE](NOTICE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
