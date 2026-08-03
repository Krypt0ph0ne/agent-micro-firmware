# Flashing Agent Micro on macOS

> [!IMPORTANT]
> **Developer Preview — source only.** Build the firmware and companion app
> locally. No firmware, native-tool, or app binary is supplied for the preview.

This procedure was tested only with a SinLoon SL2024502, ASIN `B0DN9T9J75`,
with six keys, one encoder, six LEDs, factory USB ID `1189:8890`, and a CH552G.
Visually identical devices and later revisions can have different pinouts.

> [!CAUTION]
> Flashing permanently replaces the manufacturer's application firmware. The
> original code could not be backed up, no manufacturer image is available,
> and factory restore is not possible. Recovery only reflashes Agent Micro via
> SW2; it never restores the manufacturer firmware. Read the entire procedure
> and accept this consequence before every flash operation.

## Developer Preview identity and scope

The firmware and companion macOS app currently share experimental USB ID
`4249:4287`. It was selected locally and is not an allocated public USB
identity. `1209:A6E1` has only been requested from pid.codes; it is not
assigned, not active, and must not be presented as an official allocation.

This preview distributes source only. Users build the firmware, native flash
tools, and matching app locally. No stable firmware binary will be published
unless and until an appropriate identity is assigned and applied.

Two original, redistributable photos are still required before the first
stable release:

1. board oriented with USB-C at the top and SW2 clearly identified;
2. an insulated tool bridging only the two SW2 pads, with neighboring
   components visibly clear.

The required filenames and image checklist are in `docs/images/README.md`.
Do not substitute Amazon or seller imagery.

## 1. Install and build locally

```bash
xcode-select --install
brew install sdcc

git clone https://github.com/Krypt0ph0ne/agent-micro-firmware.git
cd agent-micro-firmware

make doctor
make clean all
make tools

shasum -a 256 build/agent-micro-firmware.bin
```

`make doctor` requires SDCC 4.6.0. Build output belongs only in `build/`.
Review the displayed hash and retain it with your test notes. Do not flash a
binary downloaded from an issue, comment, or untrusted fork.

`make tools` compiles the native IOKit programs from the C++ source in this
clone. The repository contains no prebuilt flasher.

## 2. Inspect the board

Disconnect the board. Hold it with USB-C at the top. Locate the unpopulated,
two-pad `SW2` footprint. Bend a metal paperclip so that its tip can bridge
**only the two SW2 contacts**; use its insulated or non-contact end as the
handle. Do not bridge arbitrary pins or work on a powered board until the
instructions explicitly say to connect it.

If the labels or layout do not match the tested hardware description in
`HARDWARE_NOTES.md` (and the release photos once available), stop. Do not
assume the firmware is compatible.

## 3. Run the read-only preflight

Start this command before connecting the board:

```bash
sudo ./tools/run_native_preflight_sequence.sh
```

The tools need `sudo` on macOS to open the bootloader's USB device/interface
exclusively through IOKit. The wrapper resolves and executes only binaries in
this clone's `build/tools/` directory.

Now enter the protected WCH bootloader:

1. keep USB disconnected and bridge SW2;
2. connect through a reliable USB 2.0 hub or direct data cable;
3. keep SW2 bridged for 5–8 seconds;
4. release the paperclip after the terminal reports `found 4348:55e0`.

The blue LED is not a reliable bootloader indicator: it may briefly flash,
but it does not do so on every attempt. Use the terminal output, not the LED,
to decide whether the bootloader was reached.

The tool waits up to ten minutes for bootloader ID `4348:55e0`. Expected
milestones include:

```text
found 4348:55e0
Detected CH552, sub-ID 0x11
Boot version bytes=00 02 05 00
Device UID read for the volatile session key; not displayed or stored
PREFLIGHT PASSED: no erase, key setup, write, verify, config-write, reset, or exit command was sent.
```

The preflight can be stopped with Control-C or by disconnecting USB before its
final line. Its code path contains no SetKey, erase, program, verify, restart,
configuration-write, or DataFlash-write operation. Do not continue unless the
final `PREFLIGHT PASSED` line appears.

## 4. Program and verify

Re-enter the bootloader with SW2 if it timed out, then run:

> [!CAUTION]
> **This is the destructive step.** It permanently replaces the manufacturer
> application. No backup, manufacturer image, or factory-restore path exists.
> Continue only if you accept that future recovery can only flash Agent Micro
> again.

```bash
sudo ./tools/run_native_flash_sequence.sh
```

For this programming pass, bridge the two SW2 contacts again with the bent
paperclip while the board is disconnected, connect USB, and keep the contacts
shorted until the terminal begins the firmware transfer (the first `Program:`
progress line). Then remove the paperclip. Do not wait for a blue LED; it is
not shown reliably.

This wrapper passes the flasher's explicit
`--confirm-replace-factory` safeguard. It erases application code and cannot
restore the original seller application.

A successful run reports complete progress for both phases:

```text
Program: …/… bytes
Verify: …/… bytes
FLASH AND VERIFY SUCCEEDED; restart requested
```

Some macOS/bootloader combinations disconnect immediately after verification,
so the restart request can instead end with:

```text
WARNING: verified successfully, but restart command failed
```

That warning is acceptable only after the `Verify` counter completed. If
programming or verification failed, do not reconnect as a normal keyboard;
return to SW2 recovery and repeat the preflight with a trusted build.

## 5. Reconnect and identify

Disconnect USB, remove the SW2 bridge, and reconnect normally:

```bash
ioreg -p IOUSB -l -w 0 |
  grep -E 'Agent Micro|"idVendor"|"idProduct"'
```

Developer Preview builds enumerate as the experimental `4249:4287`. The
requested `1209:A6E1` identity is not assigned or active. Build a matching
source-only Agent Micro app revision that recognizes `4249:4287`.

## 6. Mandatory functional acceptance

Before trusting the device, test:

- all six keys individually;
- encoder left, right, and push (nine logical controls total);
- all six RGB LEDs, including distinct colors;
- Raw HID configuration transfer from the Agent Micro app;
- status query and physical press/release events;
- held shortcut press and release; and
- emergency release-all, followed by confirmation that no modifier remains
  logically pressed.

## Recovery

Recovery does not depend on the installed application firmware:

1. disconnect USB;
2. bridge only the two SW2 contacts with a bent paperclip while disconnected;
3. connect while holding the bridge for 5–8 seconds;
4. use the terminal, not the optional blue LED, to confirm `4348:55e0`;
5. run the read-only preflight;
6. re-enter SW2, keep the contacts shorted until `Program:` starts, then
   release the paperclip and flash/verify a trusted Agent Micro image.

This installs Agent Micro again. It does not and cannot restore the unavailable
manufacturer application.

## Troubleshooting

**Nothing appears:** Try a known data-capable cable, a direct port, or a USB
2.0 hub. Charge-only cables do not enumerate.

**Factory ID `1189:8890` appears:** SW2 was not bridged early or long enough.
Disconnect completely and repeat the cold-start sequence.

**Preview ID `4249:4287` or Agent Micro appears:** The application is running,
not the bootloader. Disconnect and repeat SW2 recovery.

**Bootloader appears but the tool times out:** The CH552 ROM command window is
short. Start the wrapper first, then enter the bootloader; avoid an
interactive delay between detection and preflight.

**Unexpected chip, sub-ID, configuration, or boot version:** Stop. The board
does not match the tested safety baseline.

**Verify fails:** Do not treat the flash as successful. Re-enter SW2 recovery,
rebuild with SDCC 4.6.0, confirm the SHA-256 and cable, and repeat. Never bypass
the identity or `0x3800` safety checks.
