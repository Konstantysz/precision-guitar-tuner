#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <Config.h>

using namespace PrecisionTuner;

TEST(ConfigTest, DefaultValues)
{
    Config config = Config::GetDefault();

    EXPECT_EQ(config.window.width, 1024);
    EXPECT_EQ(config.window.height, 768);
    EXPECT_EQ(config.audio.sampleRate, 48000);
    EXPECT_EQ(config.tuning.referencePitch, 440.0f);
}

TEST(ConfigTest, Serialization)
{
    Config config = Config::GetDefault();
    config.window.width = 1920;
    config.tuning.referencePitch = 442.0f;

    std::filesystem::path testPath = "test_config.json";

    // Save
    bool saved = config.Save(testPath);
    ASSERT_TRUE(saved);
    ASSERT_TRUE(std::filesystem::exists(testPath));

    // Load
    Config loadedConfig = Config::Load(testPath);

    EXPECT_EQ(loadedConfig.window.width, 1920);
    EXPECT_EQ(loadedConfig.tuning.referencePitch, 442.0f);

    // Cleanup
    std::filesystem::remove(testPath);
}
