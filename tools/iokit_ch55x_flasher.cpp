// SPDX-License-Identifier: MIT
// Native macOS CH552 bootloader client for the known 0x4348:0x55e0 device.
//
// Safety model:
//   --preflight <bin> performs only Detect and ReadConfig.
//   --flash <bin> --confirm-replace-factory is required for any mutation.
// The program has no config-write, data-flash erase/write, OTP, or protection-
// removal operation. It accepts only the measured CH552/sub-ID/boot version.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {

constexpr uint16_t kVendor = 0x4348;
constexpr uint16_t kProduct = 0x55e0;
constexpr size_t kMaxCodeSize = 0x3800;
constexpr size_t kCodeSectorSize = 1024;
constexpr uint32_t kMinimumEraseSectors = 8;
constexpr uint32_t kMaximumEraseSectors = 14;
constexpr UInt8 kInPipe = 1;   // descriptor endpoint 0x82
constexpr UInt8 kOutPipe = 2;  // descriptor endpoint 0x02
constexpr bool kFlashImplementationValidated = true;

struct Reply {
    UInt8 status = 0;
    std::vector<UInt8> payload;
};

uint32_t numberProperty(io_service_t service, CFStringRef key) {
    uint32_t value = 0;
    CFTypeRef property = IORegistryEntryCreateCFProperty(
        service, key, kCFAllocatorDefault, 0);
    if (property && CFGetTypeID(property) == CFNumberGetTypeID()) {
        CFNumberGetValue(static_cast<CFNumberRef>(property),
                         kCFNumberSInt32Type, &value);
    }
    if (property) CFRelease(property);
    return value;
}

uint32_t readLe32(const UInt8* bytes) {
    return static_cast<uint32_t>(bytes[0]) |
        (static_cast<uint32_t>(bytes[1]) << 8) |
        (static_cast<uint32_t>(bytes[2]) << 16) |
        (static_cast<uint32_t>(bytes[3]) << 24);
}

class Interface {
public:
    ~Interface() { close(); }

    bool open() {
        io_iterator_t iterator = IO_OBJECT_NULL;
        CFMutableDictionaryRef matching =
            IOServiceMatching("IOUSBHostInterface");
        if (!matching) return fail("cannot create interface match");
        IOReturn result = IOServiceGetMatchingServices(
            kIOMainPortDefault, matching, &iterator);
        if (result != kIOReturnSuccess)
            return failCode("IOServiceGetMatchingServices", result);

        io_service_t target = IO_OBJECT_NULL;
        while (io_service_t service = IOIteratorNext(iterator)) {
            if (numberProperty(service, CFSTR("idVendor")) == kVendor &&
                numberProperty(service, CFSTR("idProduct")) == kProduct &&
                numberProperty(service, CFSTR("bInterfaceNumber")) == 0) {
                target = service;
                break;
            }
            IOObjectRelease(service);
        }
        IOObjectRelease(iterator);
        if (!target) return fail("configured CH552 bootloader not found");

        IOCFPlugInInterface** plugin = nullptr;
        SInt32 score = 0;
        result = IOCreatePlugInInterfaceForService(
            target, kIOUSBInterfaceUserClientTypeID,
            kIOCFPlugInInterfaceID, &plugin, &score);
        IOObjectRelease(target);
        if (result != kIOReturnSuccess || !plugin)
            return failCode("IOCreatePlugInInterfaceForService", result);

        HRESULT query = (*plugin)->QueryInterface(
            plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID500),
            reinterpret_cast<LPVOID*>(&interface_));
        (*plugin)->Release(plugin);
        if (query != S_OK || !interface_)
            return fail("QueryInterface failed");

        result = (*interface_)->USBInterfaceOpen(interface_);
        if (result != kIOReturnSuccess) {
            failCode("USBInterfaceOpen", result);
            close();
            return false;
        }

        UInt8 endpointCount = 0;
        result = (*interface_)->GetNumEndpoints(interface_, &endpointCount);
        if (result != kIOReturnSuccess || endpointCount != 2) {
            fail("unexpected endpoint layout");
            close();
            return false;
        }
        return true;
    }

    bool command(UInt8 command, const std::vector<UInt8>& payload,
                 Reply& reply) {
        if (!interface_ || payload.size() > 61)
            return fail("invalid command payload");

        std::vector<UInt8> packet;
        packet.reserve(payload.size() + 3);
        packet.push_back(command);
        packet.push_back(static_cast<UInt8>(payload.size() & 0xff));
        packet.push_back(static_cast<UInt8>((payload.size() >> 8) & 0xff));
        packet.insert(packet.end(), payload.begin(), payload.end());

        IOReturn result = (*interface_)->WritePipeTO(
            interface_, kOutPipe, packet.data(),
            static_cast<UInt32>(packet.size()), 2000, 2000);
        if (result != kIOReturnSuccess)
            return failCode("bulk OUT", result);

        std::array<UInt8, 64> buffer{};
        UInt32 length = buffer.size();
        result = (*interface_)->ReadPipeTO(
            interface_, kInPipe, buffer.data(), &length, 2000, 2000);
        if (result != kIOReturnSuccess)
            return failCode("bulk IN", result);
        if (length < 4 || buffer[0] != command)
            return fail("malformed command reply");

        const UInt16 payloadLength = static_cast<UInt16>(
            buffer[2] | (buffer[3] << 8));
        if (payloadLength > 60 || length != payloadLength + 4)
            return fail("reply length mismatch");

        reply.status = buffer[1];
        reply.payload.assign(buffer.begin() + 4,
                             buffer.begin() + 4 + payloadLength);
        return true;
    }

private:
    bool fail(const char* message) {
        std::fprintf(stderr, "ERROR: %s\n", message);
        return false;
    }

    bool failCode(const char* operation, IOReturn result) {
        std::fprintf(stderr, "ERROR: %s: 0x%08x\n", operation,
                     static_cast<unsigned>(result));
        return false;
    }

    void close() {
        if (interface_) {
            (*interface_)->USBInterfaceClose(interface_);
            (*interface_)->Release(interface_);
            interface_ = nullptr;
        }
    }

    IOUSBInterfaceInterface500** interface_ = nullptr;
};

bool readFirmware(const char* path, std::vector<UInt8>& firmware) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::fprintf(stderr, "ERROR: cannot open firmware: %s\n", path);
        return false;
    }
    firmware.assign(std::istreambuf_iterator<char>(input),
                    std::istreambuf_iterator<char>());
    if (firmware.empty() || firmware.size() > kMaxCodeSize) {
        std::fprintf(stderr,
                     "ERROR: firmware size %zu is outside 1..%zu bytes\n",
                     firmware.size(), kMaxCodeSize);
        return false;
    }
    return true;
}

bool successful(const Reply& reply) {
    // Byte 1 is session-dependent on this bootloader. Mutating command
    // acknowledgements use the exact two-byte payload 00 00.
    return reply.payload == std::vector<UInt8>{0x00, 0x00};
}

bool detectAndReadConfig(Interface& usb, std::array<UInt8, 8>& uid) {
    const std::vector<UInt8> detectPayload = {
        0x52, 0x11, 'M', 'C', 'U', ' ', 'I', 'S', 'P', ' ', '&', ' ',
        'W', 'C', 'H', '.', 'C', 'N'
    };
    Reply reply;
    if (!usb.command(0xA1, detectPayload, reply)) {
        std::fprintf(stderr, "ERROR: Detect transport failed\n");
        return false;
    }
    std::printf("Detect reply: status=0x%02x payload=", reply.status);
    for (size_t i = 0; i < reply.payload.size(); ++i)
        std::printf("%02x%s", reply.payload[i],
                    i + 1 == reply.payload.size() ? "\n" : " ");
    if (reply.payload.empty()) std::putchar('\n');

    // Byte 1 of CH55x v2 replies is not a stable success status: this exact
    // device has returned both 0x01 and 0xa4 while the identifying payload
    // remained valid. Validate read replies by their exact payload instead.
    if (reply.payload.size() != 2 || reply.payload[0] != 0x52 ||
        reply.payload[1] != 0x11) {
        std::fprintf(stderr, "ERROR: device is not measured CH552/sub-ID 0x11\n");
        return false;
    }
    std::puts("Detected CH552, sub-ID 0x11");

    if (!usb.command(0xA7, {0x1f, 0x00}, reply) ||
        reply.payload.size() != 26 ||
        reply.payload[0] != 0x1f || reply.payload[1] != 0x00) {
        std::fprintf(stderr, "ERROR: full ReadConfig failed\n");
        return false;
    }

    const uint32_t cfg1 = readLe32(&reply.payload[2]);
    const uint32_t cfg2 = readLe32(&reply.payload[6]);
    const uint32_t cfg3 = readLe32(&reply.payload[10]);
    const UInt8* boot = &reply.payload[14];
    std::copy_n(reply.payload.begin() + 18, 8, uid.begin());

    std::printf("CFG1=0x%08x CFG2=0x%08x CFG3=0x%08x\n",
                cfg1, cfg2, cfg3);
    std::printf("Boot version bytes=%02x %02x %02x %02x\n",
                boot[0], boot[1], boot[2], boot[3]);
    std::puts("Device UID read for the volatile session key; not displayed or stored");

    // CFG3's upper 16 bits are a per-session value (observed 0000, 01ff,
    // and 24a4 without any write). Its measured persistent lower word is 52ff.
    if (cfg1 != 0xffffffff || cfg2 != 0x00000023 ||
        (cfg3 & 0x0000ffff) != 0x000052ff ||
        boot[0] != 0x00 || boot[1] != 0x02 ||
        boot[2] != 0x05 || boot[3] != 0x00) {
        std::fprintf(stderr,
                     "ERROR: device identity/config differs from saved baseline\n");
        return false;
    }
    return true;
}

bool setSessionKey(Interface& usb, const std::array<UInt8, 8>& uid,
                   std::array<UInt8, 8>& xorKey) {
    UInt8 checksum = 0;
    for (size_t i = 0; i < 4; ++i)
        checksum = static_cast<UInt8>(checksum + uid[i]);

    // A zero seed is the conservative, documented open-source convention.
    // The resulting key is seven UID checksums followed by checksum+chip ID.
    std::vector<UInt8> keyBase(30, 0x00);
    xorKey.fill(checksum);
    xorKey[7] = static_cast<UInt8>(checksum + 0x52);

    UInt8 expectedChecksum = 0;
    for (UInt8 byte : xorKey)
        expectedChecksum = static_cast<UInt8>(expectedChecksum + byte);

    Reply reply;
    if (!usb.command(0xA3, keyBase, reply)) {
        std::fprintf(stderr, "ERROR: bootloader session-key transport failed\n");
        return false;
    }
    std::printf("Session-key reply: status=0x%02x payload=", reply.status);
    for (size_t i = 0; i < reply.payload.size(); ++i)
        std::printf("%02x%s", reply.payload[i],
                    i + 1 == reply.payload.size() ? "\n" : " ");
    if (reply.payload.empty()) std::putchar('\n');
    std::printf("Expected XOR-key checksum=0x%02x\n", expectedChecksum);

    // This bootloader returns the derived-key checksum followed by 0x00;
    // its byte-1 session tag varies and is intentionally not trusted.
    if (reply.payload !=
        std::vector<UInt8>{expectedChecksum, static_cast<UInt8>(0x00)}) {
        std::fprintf(stderr, "ERROR: bootloader session-key setup failed\n");
        return false;
    }
    return true;
}

bool eraseCodeFlash(Interface& usb, uint32_t sectors) {
    const std::vector<UInt8> payload = {
        static_cast<UInt8>(sectors & 0xff),
        static_cast<UInt8>((sectors >> 8) & 0xff),
        static_cast<UInt8>((sectors >> 16) & 0xff),
        static_cast<UInt8>((sectors >> 24) & 0xff)
    };
    Reply reply;
    if (!usb.command(0xA4, payload, reply) || !successful(reply)) {
        std::fprintf(stderr, "ERROR: code-flash erase failed\n");
        return false;
    }
    return true;
}

bool transferImage(Interface& usb, UInt8 command,
                   const std::vector<UInt8>& original,
                   const std::array<UInt8, 8>& xorKey,
                   const char* label) {
    std::vector<UInt8> image = original;
    while (image.size() % 8) image.push_back(0x00);

    size_t offset = 0;
    while (offset < image.size()) {
        const size_t chunk = std::min<size_t>(56, image.size() - offset);
        std::vector<UInt8> payload(5 + chunk);
        payload[0] = static_cast<UInt8>(offset & 0xff);
        payload[1] = static_cast<UInt8>((offset >> 8) & 0xff);
        payload[2] = static_cast<UInt8>((offset >> 16) & 0xff);
        payload[3] = static_cast<UInt8>((offset >> 24) & 0xff);
        // Protocol padding byte. Zero is accepted by the reference clients.
        payload[4] = 0x00;
        for (size_t i = 0; i < chunk; ++i)
            payload[5 + i] = image[offset + i] ^ xorKey[i & 7];

        Reply reply;
        if (!usb.command(command, payload, reply) || !successful(reply)) {
            std::fprintf(stderr, "ERROR: %s failed at offset %zu\n",
                         label, offset);
            return false;
        }
        offset += chunk;
        std::printf("%s: %zu/%zu bytes\r", label, offset, image.size());
        std::fflush(stdout);
    }
    std::putchar('\n');
    return true;
}

void usage(const char* program) {
    std::fprintf(stderr,
        "Usage:\n"
        "  %s --preflight firmware.bin\n"
        "  %s --session-test firmware.bin\n"
        "  %s --flash firmware.bin --confirm-replace-factory\n",
        program, program, program);
}

}  // namespace

int main(int argc, char** argv) {
    const bool preflight = argc == 3 && std::string(argv[1]) == "--preflight";
    const bool sessionTest = argc == 3 &&
        std::string(argv[1]) == "--session-test";
    const bool flash = argc == 4 && std::string(argv[1]) == "--flash" &&
        std::string(argv[3]) == "--confirm-replace-factory";
    if (!preflight && !sessionTest && !flash) {
        usage(argv[0]);
        return 64;
    }
    if (flash && !kFlashImplementationValidated) {
        std::fprintf(stderr,
                     "ERROR: flash path is safety-locked pending packet-format validation\n");
        return 65;
    }

    std::vector<UInt8> firmware;
    if (!readFirmware(argv[2], firmware)) return 2;
    const uint32_t eraseSectors = std::max<uint32_t>(
        kMinimumEraseSectors,
        static_cast<uint32_t>((firmware.size() + kCodeSectorSize - 1) /
                              kCodeSectorSize));
    if (eraseSectors > kMaximumEraseSectors) {
        std::fprintf(stderr, "ERROR: calculated erase range reaches bootloader\n");
        return 2;
    }
    std::printf("Firmware validated: %zu bytes (limit %zu, bootloader starts "
                "at 0x3800)\n", firmware.size(), kMaxCodeSize);
    std::printf("Flash plan: %u x 1024-byte code sectors; DataFlash/config and "
                "bootloader excluded\n", eraseSectors);

    Interface usb;
    if (!usb.open()) return 3;
    std::array<UInt8, 8> uid{};
    if (!detectAndReadConfig(usb, uid)) return 4;

    if (preflight) {
        std::puts("PREFLIGHT PASSED: no erase, key setup, write, verify, "
                  "config-write, reset, or exit command was sent.");
        return 0;
    }

    std::array<UInt8, 8> xorKey{};
    if (sessionTest)
        std::puts("SESSION TEST: setting volatile key only; no erase or write follows");
    else
        std::puts("FLASH AUTHORIZED: setting volatile bootloader session key");
    if (!setSessionKey(usb, uid, xorKey)) return 5;
    if (sessionTest) {
        std::puts("SESSION TEST PASSED: no erase, program, verify, config-write, "
                  "reset, or exit command was sent.");
        return 0;
    }
    std::puts("Erasing CH552 code flash only (DataFlash/config untouched)");
    if (!eraseCodeFlash(usb, eraseSectors)) return 6;
    if (!transferImage(usb, 0xA5, firmware, xorKey, "Program")) return 7;
    if (!transferImage(usb, 0xA6, firmware, xorKey, "Verify")) return 8;

    Reply reply;
    if (!usb.command(0xA2, {0x01}, reply)) {
        std::fprintf(stderr,
                     "WARNING: verified successfully, but restart command failed\n");
        return 9;
    }
    std::puts("FLASH AND VERIFY SUCCEEDED; restart requested");
    return 0;
}
