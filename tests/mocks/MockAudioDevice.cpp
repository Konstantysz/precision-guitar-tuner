#include "MockAudioDevice.h"

MockAudioDevice::MockAudioDevice()
    : isOpen(false), isRunning(false), openResult(true), startResult(true), config(), callback(nullptr),
      userPtr(nullptr)
{
}

bool MockAudioDevice::Open(uint32_t deviceId,
    const GuitarIO::AudioStreamConfig &config,
    GuitarIO::AudioCallback userCallback,
    void *userPtr)
{
    this->config = config;
    this->callback = userCallback;
    this->userPtr = userPtr;
    isOpen = true;
    return openResult;
}

bool MockAudioDevice::OpenDefault(const GuitarIO::AudioStreamConfig &config,
    GuitarIO::AudioCallback userCallback,
    void *userPtr)
{
    return Open(0, config, userCallback, userPtr);
}

bool MockAudioDevice::Start()
{
    if (!isOpen)
    {
        return false;
    }
    isRunning = true;
    return startResult;
}

bool MockAudioDevice::Stop()
{
    isRunning = false;
    return true;
}

void MockAudioDevice::Close()
{
    isOpen = false;
    isRunning = false;
}

bool MockAudioDevice::IsOpen() const
{
    return isOpen;
}

bool MockAudioDevice::IsRunning() const
{
    return isRunning;
}

std::string MockAudioDevice::GetLastError() const
{
    return "Mock Error";
}

void MockAudioDevice::SetOpenResult(bool result)
{
    openResult = result;
}

void MockAudioDevice::SetStartResult(bool result)
{
    startResult = result;
}

int MockAudioDevice::TriggerCallback(std::span<const float> input, std::span<float> output)
{
    if (callback && isRunning)
    {
        return callback(input, output, userPtr);
    }
    return 0;
}

const GuitarIO::AudioStreamConfig &MockAudioDevice::GetConfig() const
{
    return config;
}
