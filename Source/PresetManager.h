#pragma once

#include <JuceHeader.h>

class PresetManager final
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState&);

    [[nodiscard]] juce::StringArray getPresetNames() const;
    [[nodiscard]] const juce::String& getCurrentPresetName() const noexcept { return currentPresetName; }
    [[nodiscard]] bool isFactoryPreset(const juce::String&) const noexcept;
    [[nodiscard]] bool isUserPreset(const juce::String&) const;

    bool loadPreset(const juce::String& name);
    bool saveUserPreset(const juce::String& name);
    bool renameUserPreset(const juce::String& newName);
    bool deleteUserPreset();
    [[nodiscard]] juce::String discover();
    void createNewFieldSeed();

private:
    void applyValue(const char* parameterId, float plainValue);
    [[nodiscard]] juce::File getPresetDirectory() const;
    [[nodiscard]] juce::File getUserPresetFile(const juce::String& name) const;
    [[nodiscard]] static juce::String sanitiseName(const juce::String& name);

    juce::AudioProcessorValueTreeState& state;
    juce::String currentPresetName { "INIT" };
};
