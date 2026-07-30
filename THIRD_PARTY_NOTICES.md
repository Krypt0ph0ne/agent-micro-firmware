# Third-party notices

## CH552-Macropad-mini

Parts of the firmware are derived from Stefan Wagner's
[CH552-Macropad-mini](https://github.com/wagiminator/CH552-Macropad-mini),
licensed under CC BY-SA 3.0. Agent Micro's changes are summarized in
`NOTICE`.

## chprog.py

`tools/chprog.py` is Stefan Wagner's `chprog` utility and retains its MIT
license. The corresponding license text is in
`LICENSES/chprog-MIT.txt`. It is included as a portable reference tool; the
documented macOS procedure uses the locally compiled native tools.

## Native macOS flash and diagnostic tools

The source files `tools/iokit_*.cpp` are distributed under the MIT License,
found in `LICENSES/native-tools-MIT.txt`. Generated executables are never
stored in this repository.

## CH554 register header

`include/ch554.h` contains the WCH copyright notice present in the file. The
byte-equivalent community source used for this repository was obtained from
[dsm/USB-Blaster_CH552](https://github.com/dsm/USB-Blaster_CH552), whose
repository declares the MIT License; that license is preserved in
`LICENSES/USB-Blaster_CH552-MIT.txt`.

This provenance record does not remove or replace WCH's original copyright
notice. If a clearer upstream licensing statement becomes available, this
record should be updated before the next release.
