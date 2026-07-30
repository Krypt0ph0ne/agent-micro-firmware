// SPDX-License-Identifier: MIT
// Read-only IOKit probe for the configured CH55x bootloader interface.
// It opens the interface and reads pipe metadata only; no USB payload is sent.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <cstdio>

namespace {
constexpr uint16_t kVendor = 0x4348;
constexpr uint16_t kProduct = 0x55e0;

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

void printResult(const char* operation, IOReturn result) {
    std::printf("%s: 0x%08x%s\n", operation,
                static_cast<unsigned>(result),
                result == kIOReturnSuccess ? " (success)" : "");
}
}  // namespace

int main() {
    io_iterator_t iterator = IO_OBJECT_NULL;
    CFMutableDictionaryRef matching = IOServiceMatching("IOUSBHostInterface");
    if (!matching) return 1;
    IOReturn result = IOServiceGetMatchingServices(
        kIOMainPortDefault, matching, &iterator);
    if (result != kIOReturnSuccess) {
        printResult("IOServiceGetMatchingServices", result);
        return 2;
    }

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
    if (!target) {
        std::fprintf(stderr, "configured CH55x interface not found\n");
        return 3;
    }

    std::printf("found interface: location=0x%08x class=0x%02x "
                "subclass=0x%02x protocol=0x%02x endpoints=%u\n",
                numberProperty(target, CFSTR("locationID")),
                numberProperty(target, CFSTR("bInterfaceClass")),
                numberProperty(target, CFSTR("bInterfaceSubClass")),
                numberProperty(target, CFSTR("bInterfaceProtocol")),
                numberProperty(target, CFSTR("bNumEndpoints")));

    IOCFPlugInInterface** plugin = nullptr;
    SInt32 score = 0;
    result = IOCreatePlugInInterfaceForService(
        target, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID,
        &plugin, &score);
    IOObjectRelease(target);
    printResult("IOCreatePlugInInterfaceForService", result);
    if (result != kIOReturnSuccess || !plugin) return 4;

    IOUSBInterfaceInterface500** interface = nullptr;
    HRESULT query = (*plugin)->QueryInterface(
        plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID500),
        reinterpret_cast<LPVOID*>(&interface));
    std::printf("QueryInterface: 0x%08x%s\n", static_cast<unsigned>(query),
                query == S_OK ? " (success)" : "");
    (*plugin)->Release(plugin);
    if (query != S_OK || !interface) return 5;

    result = (*interface)->USBInterfaceOpen(interface);
    printResult("USBInterfaceOpen", result);
    if (result != kIOReturnSuccess) {
        (*interface)->Release(interface);
        return 6;
    }

    UInt8 count = 0;
    result = (*interface)->GetNumEndpoints(interface, &count);
    printResult("GetNumEndpoints", result);
    std::printf("pipe_count=%u\n", count);

    for (UInt8 pipe = 1; pipe <= count; ++pipe) {
        UInt8 direction = 0, number = 0, transferType = 0, interval = 0;
        UInt16 maxPacketSize = 0;
        result = (*interface)->GetPipeProperties(
            interface, pipe, &direction, &number, &transferType,
            &maxPacketSize, &interval);
        printResult("GetPipeProperties", result);
        std::printf("pipe=%u direction=%u endpoint=%u transfer_type=%u "
                    "max_packet=%u interval=%u\n",
                    pipe, direction, number, transferType,
                    maxPacketSize, interval);
    }

    result = (*interface)->USBInterfaceClose(interface);
    printResult("USBInterfaceClose", result);
    (*interface)->Release(interface);
    std::puts("probe complete; no USB payload and no flash data sent");
    return 0;
}
