# Developer Preview checklist

Use this checklist for the firmware and companion macOS app together. The
target is a clearly labeled **Developer Preview — source only**, not a stable
binary release.

## Publication scope

- Both repositories label the same source-only Developer Preview.
- Users build the firmware, native flash tools, and app locally.
- No firmware binary, prebuilt native flasher, stable release, or stable-release
  claim is published for the preview.
- Firmware and app both recognize the active experimental USB ID `4249:4287`.
- `4249:4287` is described as locally selected and not publicly allocated.
- `1209:A6E1` is described only as requested, not assigned or active.
- No wording implies an official USB allocation or claims another party's USB
  identity as Agent Micro's own.

## Build and consistency

- Run `make doctor`, `make test`, and `git diff --check` in the firmware clone.
- Confirm generated files remain under ignored `build/` and are not staged.
- Build a matching companion app revision from source and verify that it finds
  firmware enumerating as `4249:4287`.
- Confirm the app and firmware still agree on Raw HID protocol v2, `CP` magic,
  32-byte reports, and the nine logical controls.

## Before every flash

- Confirm the board is the tested SinLoon SL2024502 revision and inspect SW2.
- Accept that flashing permanently replaces the manufacturer application.
- Accept that no manufacturer backup/image or factory restore is available.
- Understand that recovery exclusively means using SW2 to flash Agent Micro
  again.
- Build locally, record the firmware SHA-256, and run the read-only preflight.
- Proceed with the destructive command only after `PREFLIGHT PASSED`.

## Functional acceptance

- Test all six keys, encoder left/right/push, all six RGB LEDs, held shortcuts,
  Raw HID status/events/configuration, and emergency release-all.
- Verify the source-built companion app connects to `4249:4287` and exercises
  the expected preview functionality.
- Record the exact app and firmware commits used for the paired test.
