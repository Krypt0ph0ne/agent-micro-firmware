# pid.codes request draft

Status: **not submitted**. Stable firmware binaries remain blocked until the
request is accepted.

- Organization: `Krypt0ph0ne`
- Device: `Agent Micro`
- Requested VID: `0x1209`
- Candidate PID: `0xA6E1` (must be rechecked immediately before submission)
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

Submission checklist:

- repository is public and all source/license files are present;
- no device UID, DataFlash backup, factory dump, or prebuilt native executable;
- candidate PID is still unallocated in the current pid.codes list;
- add/update the `Krypt0ph0ne` organization page and Agent Micro device entry;
- submit a pull request to `pid.codes/pidcodes.github.com`;
- replace the experimental identity only after merge; and
- keep app compatibility with `4249:4287` for at least one transition release.
