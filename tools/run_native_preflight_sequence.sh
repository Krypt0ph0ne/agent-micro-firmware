#!/bin/bash
# SPDX-License-Identifier: CC-BY-SA-3.0
set -euo pipefail

# Read-only end-to-end validation for the exact test image and measured CH552.
# The flasher's --preflight branch cannot execute SetKey, Erase, Program,
# Verify, restart, config-write, or DataFlash-write operations.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FIRMWARE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG_PROBE="$FIRMWARE_DIR/build/tools/iokit_usb_config_probe"
FLASHER="$FIRMWARE_DIR/build/tools/iokit_ch55x_flasher"
FIRMWARE="$FIRMWARE_DIR/build/agent-micro-firmware.bin"

if [[ ! -x "$CONFIG_PROBE" || ! -x "$FLASHER" || ! -f "$FIRMWARE" ]]; then
  echo "Build output missing. Run 'make all tools' first." >&2
  exit 1
fi

"$CONFIG_PROBE"
exec "$FLASHER" --preflight "$FIRMWARE"
