// SPDX-License-Identifier: MIT
// Read-only/volatile USB configuration probe for the CH55x ROM bootloader.
// This does not send any WCH protocol command and cannot erase or program flash.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <cstdio>
#include <libkern/OSByteOrder.h>
#include <unistd.h>

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
    io_service_t target = IO_OBJECT_NULL;
    IOReturn result = kIOReturnSuccess;
    std::puts("waiting up to 10 minutes for CH55x bootloader 4348:55e0 ...");
    std::fflush(stdout);

    for (unsigned attempt = 0; attempt < 2400 && !target; ++attempt) {
        io_iterator_t iterator = IO_OBJECT_NULL;
        CFMutableDictionaryRef matching = IOServiceMatching("IOUSBHostDevice");
        if (!matching) {
            std::fprintf(stderr, "Could not create IOUSBHostDevice match\n");
            return 1;
        }

        result = IOServiceGetMatchingServices(
            kIOMainPortDefault, matching, &iterator);
        if (result != kIOReturnSuccess) {
            printResult("IOServiceGetMatchingServices", result);
            return 2;
        }

        while (io_service_t service = IOIteratorNext(iterator)) {
            const uint32_t vendor = numberProperty(service, CFSTR("idVendor"));
            const uint32_t product = numberProperty(service, CFSTR("idProduct"));
            if (vendor == kVendor && product == kProduct) {
                target = service;
                break;
            }
            IOObjectRelease(service);
        }
        IOObjectRelease(iterator);
        if (!target) usleep(250000);
    }

    if (!target) {
        std::fprintf(stderr, "timeout: CH55x bootloader 4348:55e0 not found\n");
        return 3;
    }

    const uint32_t location = numberProperty(target, CFSTR("locationID"));
    std::printf("found 4348:55e0 at location 0x%08x\n", location);

    IOCFPlugInInterface** plugin = nullptr;
    SInt32 score = 0;
    result = IOCreatePlugInInterfaceForService(
        target, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
        &plugin, &score);
    IOObjectRelease(target);
    printResult("IOCreatePlugInInterfaceForService", result);
    if (result != kIOReturnSuccess || !plugin) return 4;

    IOUSBDeviceInterface500** device = nullptr;
    HRESULT query = (*plugin)->QueryInterface(
        plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID500),
        reinterpret_cast<LPVOID*>(&device));
    std::printf("QueryInterface: 0x%08x%s\n", static_cast<unsigned>(query),
                query == S_OK ? " (success)" : "");
    (*plugin)->Release(plugin);
    if (query != S_OK || !device) return 5;

    result = (*device)->USBDeviceOpen(device);
    printResult("USBDeviceOpen", result);
    if (result != kIOReturnSuccess) {
        result = (*device)->USBDeviceOpenSeize(device);
        printResult("USBDeviceOpenSeize", result);
    }
    if (result != kIOReturnSuccess) {
        (*device)->Release(device);
        return 6;
    }

    UInt8 configurationCount = 0;
    result = (*device)->GetNumberOfConfigurations(device, &configurationCount);
    printResult("GetNumberOfConfigurations", result);
    std::printf("configuration_count=%u\n", configurationCount);

    if (configurationCount > 0) {
        IOUSBConfigurationDescriptorPtr descriptor = nullptr;
        result = (*device)->GetConfigurationDescriptorPtr(
            device, 0, &descriptor);
        printResult("GetConfigurationDescriptorPtr", result);
        if (result == kIOReturnSuccess && descriptor) {
            const UInt8 availableConfiguration = descriptor->bConfigurationValue;
            const UInt16 totalLength = USBToHostWord(descriptor->wTotalLength);
            std::printf("descriptor_configuration=%u interfaces=%u\n",
                        availableConfiguration, descriptor->bNumInterfaces);
            std::printf("descriptor_total_length=%u raw=", totalLength);
            const auto* bytes = reinterpret_cast<const UInt8*>(descriptor);
            for (UInt16 i = 0; i < totalLength; ++i) {
                std::printf("%02x%s", bytes[i],
                            i + 1 == totalLength ? "\n" : " ");
            }

            // Intentionally do not call GetConfiguration: this CH552 ROM
            // bootloader stalls that request. Set only the configuration
            // value explicitly advertised in its cached descriptor.
            if (availableConfiguration != 0) {
                result = (*device)->SetConfiguration(
                    device, availableConfiguration);
                printResult("SetConfiguration(direct, no prior GET)", result);
            }
        }
    }

    result = (*device)->USBDeviceClose(device);
    printResult("USBDeviceClose", result);
    (*device)->Release(device);
    std::puts("probe complete; no WCH command and no flash data sent");
    return 0;
}
