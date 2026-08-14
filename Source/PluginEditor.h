#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include <array>
#include <deque>
#include <memory>
#include <vector>

class WavoriaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    WavoriaLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosition, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
};

class ParameterKnob final : public juce::Component
{
public:
    ParameterKnob(juce::AudioProcessorValueTreeState&, const juce::String& parameterId,
                  const juce::String& displayName, const juce::String& suffix = {});
    void setAccent(juce::Colour colour) { slider.setColour(juce::Slider::rotarySliderFillColourId, colour); }
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider slider;
    juce::String name;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attachment;
};

class TerrainGlobe final : public juce::Component,
                           private juce::Timer
{
public:
    explicit TerrainGlobe(WavoriaAudioProcessor&);
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    [[nodiscard]] wavoria::dsp::TerrainParameters readTerrainParameters() const noexcept;
    [[nodiscard]] juce::Point<float> project(float x, float y, float height,
                                             float rotation, float radius) const noexcept;
    [[nodiscard]] bool isFrontFacing(float x, float y, float rotation) const noexcept;

    WavoriaAudioProcessor& processor;
    std::array<std::deque<juce::Point<float>>, WavoriaAudioProcessor::maximumVoices> histories;
    float animation { 0.0f };
};

class WavoriaAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit WavoriaAudioProcessorEditor(WavoriaAudioProcessor&);
    ~WavoriaAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ParameterKnob* addKnob(std::vector<ParameterKnob*>& group, const char* id,
                           const char* name, const char* suffix = "");
    static void layoutGrid(juce::Rectangle<int>, const std::vector<ParameterKnob*>&, int columns);
    void drawPanel(juce::Graphics&, juce::Rectangle<int>, const juce::String& title) const;

    WavoriaAudioProcessor& processor;
    WavoriaLookAndFeel lookAndFeel;
    TerrainGlobe globe;
    std::vector<std::unique_ptr<ParameterKnob>> ownedKnobs;
    std::vector<ParameterKnob*> surfaceKnobs;
    std::vector<ParameterKnob*> fieldKnobs;
    std::vector<ParameterKnob*> orbitKnobs;
    std::vector<ParameterKnob*> voiceKnobs;
    std::vector<ParameterKnob*> outputKnobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavoriaAudioProcessorEditor)
};
