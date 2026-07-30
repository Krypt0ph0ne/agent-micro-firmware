# SPDX-License-Identifier: CC-BY-SA-3.0

SKETCH      := macropad_test.c
TARGET      := agent-micro-firmware
INCLUDE_DIR := include
BUILD_DIR   := build
TOOLS_DIR   := $(BUILD_DIR)/tools

FREQ_SYS  := 16000000
XRAM_SIZE := 0x0300
XRAM_LOC  := 0x0100
CODE_SIZE := 0x3800

CC       := sdcc
OBJCOPY  := sdobjcopy
PACK_HEX := packihx
CXX      := clang++

CFLAGS  := -mmcs51 --model-small --no-xinit-opt --fconst-code
CFLAGS  += --xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) --code-size $(CODE_SIZE)
CFLAGS  += -I$(INCLUDE_DIR) -DFREQ_SYS=$(FREQ_SYS)

FIRMWARE_CFILES := $(SKETCH) $(wildcard $(INCLUDE_DIR)/*.c)
FIRMWARE_OBJECTS := $(BUILD_DIR)/macropad_test.rel \
	$(patsubst $(INCLUDE_DIR)/%.c,$(BUILD_DIR)/%.rel,$(wildcard $(INCLUDE_DIR)/*.c))

TOOL_NAMES := iokit_ch55x_flasher iokit_usb_config_probe \
	iokit_usb_interface_probe
TOOL_BINARIES := $(addprefix $(TOOLS_DIR)/,$(TOOL_NAMES))
MACOS_FRAMEWORKS := -framework IOKit -framework CoreFoundation

.PHONY: all bin hex tools doctor test clean help size

help:
	@echo "make doctor  verify the pinned local toolchain"
	@echo "make all     build BIN and HEX under build/"
	@echo "make tools   compile native macOS flashing tools from source"
	@echo "make test    build and validate size, scripts, and checksums"
	@echo "make clean   remove generated build/"

all: doctor bin hex size

bin: $(BUILD_DIR)/$(TARGET).bin

hex: $(BUILD_DIR)/$(TARGET).hex

tools: $(TOOL_BINARIES)

$(BUILD_DIR):
	@mkdir -p $@

$(TOOLS_DIR):
	@mkdir -p $@

$(BUILD_DIR)/macropad_test.rel: macropad_test.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.rel: $(INCLUDE_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).ihx: $(FIRMWARE_OBJECTS)
	$(CC) $(FIRMWARE_OBJECTS) $(CFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).ihx
	$(OBJCOPY) -I ihex -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).ihx
	$(PACK_HEX) $< > $@

$(TOOLS_DIR)/%: tools/%.cpp | $(TOOLS_DIR)
	$(CXX) -std=c++17 -O2 -Wall -Wextra $< $(MACOS_FRAMEWORKS) -o $@

doctor:
	@command -v $(CC) >/dev/null || { echo "SDCC is missing. Run: brew install sdcc" >&2; exit 1; }
	@$(CC) --version | head -n 1 | grep -q '4\.6\.0' || { echo "SDCC 4.6.0 is required for the reproducible build." >&2; exit 1; }
	@command -v $(OBJCOPY) >/dev/null || { echo "sdobjcopy is missing." >&2; exit 1; }
	@command -v $(PACK_HEX) >/dev/null || { echo "packihx is missing." >&2; exit 1; }
	@xcode-select -p >/dev/null 2>&1 || { echo "Xcode Command Line Tools are missing." >&2; exit 1; }
	@test -f LICENSE -a -f NOTICE -a -f THIRD_PARTY_NOTICES.md
	@echo "Toolchain ready: $$($(CC) --version | head -n 1)"

size: $(BUILD_DIR)/$(TARGET).bin
	@bytes=$$(wc -c < "$<" | tr -d '[:space:]'); \
	  test "$$bytes" -lt $$((0x3800)) || { echo "Firmware exceeds 0x3800: $$bytes bytes" >&2; exit 1; }; \
	  echo "FLASH: $$bytes / $$((0x3800)) bytes"

test: clean all tools
	@bash -n tools/run_native_preflight_sequence.sh \
	  tools/run_native_flash_sequence.sh
	@shasum -a 256 $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	@echo "Firmware checks passed."

clean:
	rm -rf $(BUILD_DIR)
