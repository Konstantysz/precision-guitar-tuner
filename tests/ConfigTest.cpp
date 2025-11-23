#include <Config.h>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace PrecisionTuner;

void TestDefaultConfig()
{
    Config config = Config::GetDefault();
    
    assert(config.window.width == 1024);
    assert(config.window.height == 768);
    assert(config.audio.sampleRate == 48000);
    assert(config.tuning.referencePitch == 440.0f);
    
    std::cout << "TestDefaultConfig passed" << std::endl;
}

void TestConfigSerialization()
{
    Config config = Config::GetDefault();
    config.window.width = 1920;
    config.tuning.referencePitch = 442.0f;
    
    std::filesystem::path testPath = "test_config.json";
    
    // Save
    bool saved = config.Save(testPath);
    assert(saved);
    assert(std::filesystem::exists(testPath));
    
    // Load
    Config loadedConfig = Config::Load(testPath);
    
    assert(loadedConfig.window.width == 1920);
    assert(loadedConfig.tuning.referencePitch == 442.0f);
    
    // Cleanup
    std::filesystem::remove(testPath);
    
    std::cout << "TestConfigSerialization passed" << std::endl;
}

int main()
{
    std::cout << "Running Config Tests..." << std::endl;
    
    TestDefaultConfig();
    TestConfigSerialization();
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
