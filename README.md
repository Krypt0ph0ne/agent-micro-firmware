# Agent Micro firmware

Open firmware for the tested SinLoon SL2024502 six-key, one-encoder CH552
macropad sold under ASIN `B0DN9T9J75`.

> [!IMPORTANT]
> **Developer Preview — source only.** Build the firmware and companion app
> locally. This repository does not distribute firmware, native-tool, or app
> binaries for the preview.

> [!CAUTION]
> Flashing permanently replaces the manufacturer's application firmware. No
> backup or manufacturer firmware image is available, so factory restore is
> not possible. Recovery means entering the WCH bootloader with SW2 and
> flashing Agent Micro again. Confirm that this is acceptable before every
> flash operation. Visually identical revisions can use different wiring;
> verify the model, USB identities, and SW2 location first.

Agent Micro is an independent community project. It is not an official
SinLoon, Amazon, OpenAI, or Anthropic product. This repository licenses only
the firmware and software—not the commercial PCB, enclosure, or hardware.

## Developer Preview status

The firmware and companion macOS app are being prepared together as a public
**Developer Preview — source only**. Users build both components locally and
flash at their own risk. The shared experimental identity is `4249:4287`; it
was selected locally and is not an allocated public USB identity.

VID:PID `1209:A6E1` is only
[requested from pid.codes](https://github.com/pidcodes/pidcodes.github.com/pull/1255).
It has not been assigned and is not active in either component. The request
must not be described as an official USB allocation. No stable firmware binary
will be published unless and until an appropriate identity is assigned and
applied.

| Mode | VID:PID |
| --- | --- |
| Factory application | `1189:8890` |
| WCH ROM bootloader | `4348:55e0` |
| Developer Preview firmware (active, experimental) | `4249:4287` |
| Requested only; not assigned or active | `1209:A6E1` |

The companion [Agent Micro macOS app](https://github.com/Krypt0ph0ne/agent-micro)
is part of the same source-only preview. Build a matching app revision that
recognizes the active experimental `4249:4287` identity; do not configure the
app for the requested `1209:A6E1` identity yet.

Maintainers can use the joint
[Developer Preview checklist](docs/developer-preview-checklist.md) for build,
identity, flash-safety, and app/firmware acceptance checks.

## Supported hardware

The tested revision has USB-C at the top, six keys, one rotary encoder, six
addressable RGB LEDs, and a CH552G. Its verified pin and LED maps are in
[HARDWARE_NOTES.md](HARDWARE_NOTES.md). Do not infer compatibility from the
Amazon listing or enclosure alone.

### Every other board is unsupported

The maintainers can vouch for exactly one board. Treat anything else as
incompatible until it has been measured and proven on that specific hardware.

**A different control count cannot work.** More or fewer keys, a missing
encoder, or a second encoder means this firmware will not run usefully on the
device. `ACTION_CONTROL_COUNT`, the GPIO map, and the six-pixel LED chain are
compile-time constants; a mismatch is a non-functional device, not a partially
working one.

**A matching layout is still not expected to work.** Another six-key,
one-encoder pad may use a different controller, a different pin assignment, a
different LED order, or different SW2 pads. If one turns out to be wired
identically, treat that as luck rather than as compatibility. The preflight
gate checks the CH552 chip ID, sub-ID, boot version, and fuse configuration —
it cannot tell one CH552-based *product* from another, so passing preflight is
not evidence that the pinout matches.

Adapting the firmware to different hardware means **porting** it: re-measure
every entry in the table in [HARDWARE_NOTES.md](HARDWARE_NOTES.md) and change
the corresponding definitions. That is development work with an oscilloscope or
multimeter and a board you are willing to lose. This project cannot verify the
result, and a successful port to one board says nothing about the next.

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

These are local build products, not distributed preview or stable binaries.

`make doctor` rejects an SDCC version other than 4.6.0. `make test` performs a
clean build, enforces the `0x3800` application limit, builds the native tools,
checks shell syntax, and prints SHA-256 hashes.

## Flashing and recovery

Read the complete macOS procedure before bridging or flashing anything:
[docs/flashing-macos.md](docs/flashing-macos.md).

The short command sequence is:

> [!CAUTION]
> The flash command below permanently removes the manufacturer application.
> There is no factory restore. Run it only after accepting that all recovery
> paths reflash Agent Micro rather than restore the original firmware.

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
The complete guide repeats the irreversible warning immediately before the
destructive command and documents the SW2-only recovery path.

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
