#pragma once

#include <JuceHeader.h>

#include "dsp/WaveTerrainEngine.h"

#include <array>
#include <atomic>
#include <cstdint>

class WavoriaAudioProcessor final : public juce::AudioProcessor
{
public:
    static constexpr int maximumVoices = 12;

    struct VisualVoice
    {
        float x { 0.0f };
        float y { 0.0f };
        float energy { 0.0f };
        bool active { false };
    };

    WavoriaAudioProcessor();
    ~WavoriaAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    [[nodiscard]] VisualVoice getVisualVoice(int index) const noexcept;
    [[nodiscard]] float getVisualFieldHeight(float x, float y) const noexcept;
    [[nodiscard]] float getFieldEnergy() const noexcept { return fieldEnergy.load(std::memory_order_relaxed); }
    [[nodiscard]] int getActiveVoiceCount() const noexcept { return activeVoiceCount.load(std::memory_order_relaxed); }
    void requestNewField() noexcept { clearFieldRequested.store(true, std::memory_order_release); }
    void beginMidiLearn(const juce::String& parameterId) noexcept;
    void cancelMidiLearn(const juce::String& parameterId) noexcept;
    void clearMidiLearn(const juce::String& parameterId) noexcept;
    [[nodiscard]] int getMidiControllerForParameter(const juce::String& parameterId) const noexcept;
    [[nodiscard]] bool isMidiLearning(const juce::String& parameterId) const noexcept;

    juce::AudioProcessorValueTreeState parameters;

private:
    struct VoiceSlot
    {
        wavoria::dsp::WaveTerrainReader reader;
        int note { -1 };
        int channel { 1 };
        std::uint64_t age { 0 };
        float pan { 0.0f };
        float notePressure { 0.0f };
        bool keyDown { false };
        bool sustained { false };
    };

    struct ParameterSnapshot
    {
        wavoria::dsp::TerrainParameters terrain;
        float width { 0.72f };
        float levelDb { -12.0f };
        std::uint32_t seed { 1979u };
    };

    struct AtomicVisualVoice
    {
        std::atomic<float> x { 0.0f };
        std::atomic<float> y { 0.0f };
        std::atomic<float> energy { 0.0f };
        std::atomic<bool> active { false };
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    [[nodiscard]] static int midiLearnIndexFor(const juce::String& parameterId) noexcept;
    [[nodiscard]] ParameterSnapshot readParameters() const noexcept;
    void renderRange(juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                     const ParameterSnapshot& snapshot) noexcept;
    void handleMidiMessage(const juce::MidiMessage&, const ParameterSnapshot&) noexcept;
    void startNote(int note, int channel, float velocity, const ParameterSnapshot&) noexcept;
    void stopNote(int note, int channel, bool allowTail) noexcept;
    void updateChannelPitch(int channel) noexcept;
    void releaseSustainedVoices(int channel) noexcept;
    void handleMidiControllerLearn(int controller, int value) noexcept;
    [[nodiscard]] VoiceSlot& findVoiceToUse() noexcept;
    [[nodiscard]] float frequencyFor(const VoiceSlot&) const noexcept;

    std::array<VoiceSlot, maximumVoices> voices;
    std::array<float, 16> channelPitchBend {};
    std::array<float, 16> channelPressure {};
    std::array<float, 16> channelModWheel {};
    std::array<bool, 16> sustainPedal {};
    std::array<AtomicVisualVoice, maximumVoices> visuals;
    std::array<std::atomic<float>, wavoria::dsp::SharedTerrainField::gridSize
                                   * wavoria::dsp::SharedTerrainField::gridSize> fieldVisuals {};
    wavoria::dsp::SharedTerrainField sharedField;
    juce::dsp::Gain<float> outputGain;
    std::atomic<float> fieldEnergy { 0.0f };
    std::atomic<int> activeVoiceCount { 0 };
    std::atomic<bool> clearFieldRequested { false };
    static constexpr std::array<const char*, 21> midiLearnParameterIds {
        "topology", "contour", "fold", "symmetry",
        "orbit", "radius", "rotation", "drift", "interaction",
        "deform", "memory", "gravity",
        "attack", "decay", "sustain", "release",
        "tone", "drive", "width", "level", "seed"
    };
    std::array<juce::RangedAudioParameter*, midiLearnParameterIds.size()> midiLearnParameters {};
    std::array<std::atomic<int>, midiLearnParameterIds.size()> midiCcAssignments {};
    std::atomic<int> midiLearnTarget { -1 };
    std::uint64_t voiceAge { 0 };
    int visualUpdateCountdown { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavoriaAudioProcessor)
};
