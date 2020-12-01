#include "unittest.h"

#include <map>
#include <iostream>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

namespace cf {
class String {
public:
    ~String()
    {
        if (this->stringRef) {
            CFRelease(this->stringRef);
            this->stringRef = nullptr;
        }
    }

    CFStringRef stringRef{ nullptr };

    std::string stdString()
    {
        std::string value;

        CFIndex length = CFStringGetLength(this->stringRef);
        CFIndex max_size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
        auto buffer = std::vector<char>(max_size);
        if (!CFStringGetCString(this->stringRef, buffer.data(), max_size, kCFStringEncodingUTF8)) {
            return std::string();
        }

        return std::string(buffer.data());
    }
};
}

UNITTEST(audiodev_list)
{
    UInt32 io_size{};
    OSStatus os_err{};

    AudioObjectPropertyAddress prop_address = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    os_err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &prop_address, 0, NULL, &io_size);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    auto deviceIds = std::vector<AudioObjectID>();
    deviceIds.resize(io_size / sizeof(AudioObjectID));
    os_err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &prop_address, 0, NULL, &io_size, deviceIds.data());
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    for (auto deviceId : deviceIds) {

        cf::String deviceName;

        prop_address.mSelector = kAudioObjectPropertyName;
        prop_address.mScope = kAudioObjectPropertyScopeGlobal;
        prop_address.mElement = kAudioObjectPropertyElementMaster;
        io_size = sizeof(CFStringRef);

        os_err = AudioObjectGetPropertyData(deviceId, &prop_address, 0, NULL, &io_size, &deviceName.stringRef);
        if (os_err != 0) {
            std::cout << "Error:" << os_err << std::endl;
            continue;
        }

        std::cout << "id=" <<deviceId << " name=" << deviceName.stdString() << std::endl;
    }
}

const AudioObjectPropertyAddress kDeviceChangePropertyAddress = {
    kAudioHardwarePropertyDefaultOutputDevice,
    //kAudioHardwarePropertyPlugInList,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
    };

OSStatus OnDefaultDeviceChanged(AudioObjectID object, UInt32 num_addresses,const AudioObjectPropertyAddress addresses[], void* context)
{
  if (object != kAudioObjectSystemObject)
    return noErr;

   std::cout << "OnDefaultDeviceChanged" << std::endl;

  for (UInt32 i = 0; i < num_addresses; ++i) 
  {
    if (addresses[i].mSelector == kDeviceChangePropertyAddress.mSelector &&
        addresses[i].mScope == kDeviceChangePropertyAddress.mScope &&
        addresses[i].mElement == kDeviceChangePropertyAddress.mElement &&
        context) 
        {
            std::cout << "OnDefaultDeviceChanged" << std::endl;
        }
  }

  return noErr;
}

UNITTEST(audiodev_listen)
{
    OSStatus result = AudioObjectAddPropertyListener(
      kAudioObjectSystemObject, &kDeviceChangePropertyAddress,
      &OnDefaultDeviceChanged, NULL);

    if (result != noErr) {
        std::cout << "AudioObjectAddPropertyListener() failed!";
        return;
    }
}


UNITTEST(audiodev_getdef)
{
    UInt32 io_size{};
    OSStatus os_err{};

    AudioObjectPropertyAddress prop_address = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    os_err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &prop_address, 0, NULL, &io_size);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    std::cout << io_size << std::endl;

    AudioObjectID deviceId;

    os_err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &prop_address, 0, NULL, &io_size, &deviceId);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    cf::String deviceName;

    prop_address.mSelector = kAudioObjectPropertyName;
    prop_address.mScope = kAudioObjectPropertyScopeGlobal;
    prop_address.mElement = kAudioObjectPropertyElementMaster;
    io_size = sizeof(CFStringRef);

    os_err = AudioObjectGetPropertyData(deviceId, &prop_address, 0, NULL, &io_size, &deviceName.stringRef);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    std::cout << "id=" <<deviceId << " name=" << deviceName.stdString() << std::endl;
}

UNITTEST(audiodev_setdef)
{
    UInt32 io_size{};
    OSStatus os_err{};

    if (argc != 1)
    {
        std::cout << "audiodev_setdef  audiodeviceid" << std::endl;
        return;
    }

    AudioObjectID id = atoi(argv[0]);

    AudioObjectPropertyAddress prop_address = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    os_err = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &prop_address, 0, NULL, &io_size);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }

    std::cout << io_size << std::endl;

    auto deviceIds = std::vector<AudioObjectID>();
    deviceIds.resize(io_size / sizeof(AudioObjectID));
    os_err = AudioObjectSetPropertyData(kAudioObjectSystemObject, &prop_address, 0, NULL, io_size, &id);
    if (os_err != 0) {
        std::cout << "Error:" << os_err << std::endl;
        return;
    }
}

