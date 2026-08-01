# pid.codes request

Status: **submitted on July 30, 2026** as
[pidcodes/pidcodes.github.com#1255](https://github.com/pidcodes/pidcodes.github.com/pull/1255).
The request is not an assignment: `1209:A6E1` is not active and must not be
described as an official allocation. Stable firmware binaries remain blocked
unless and until an appropriate identity is assigned and applied.

The Developer Preview of the firmware and companion app remains source only
and uses the locally selected experimental identity `4249:4287`. That identity
is not an allocated public USB identity.

- Organization: `Krypt0ph0ne`
- Device: `Agent Micro`
- Requested VID: `0x1209`
- Requested PID: `0xA6E1` (unallocated when checked before submission)
- Firmware source:
  `https://github.com/Krypt0ph0ne/agent-micro-firmware`
- Companion source:
  `https://github.com/Krypt0ph0ne/agent-micro`
- Firmware license: CC BY-SA 3.0

Proposed disclosure:

> Agent Micro is open firmware and a source-built macOS companion app for a
> commercially sold CH552 six-key macropad. The firmware, USB descriptors,
> protocol, and flashing tools are public. The project does not manufacture
> the board and does not have PCB or enclosure design files; those hardware
> designs are not claimed as open source.

Submission record:

- repository is public and all source/license files are present;
- no device UID, DataFlash backup, factory dump, or prebuilt native executable;
- `0xA6E1` was unallocated in the current pid.codes list at submission time;
- the `Krypt0ph0ne` organization page and Agent Micro device entry are included;
- the official pid.codes validator passes;
- the request commit includes a DCO sign-off;
- do not use or advertise `1209:A6E1` as assigned before the request is
  accepted;
- replace the experimental identity only after assignment and coordinated
  firmware/app changes; and
- keep app compatibility with `4249:4287` for at least one transition release.
