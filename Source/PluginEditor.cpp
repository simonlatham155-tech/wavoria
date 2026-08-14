#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace palette
{
const auto ink = juce::Colour::fromRGB(6, 10, 11);
const auto panel = juce::Colour::fromRGB(13, 22, 22);
const auto panelEdge = juce::Colour::fromRGB(43, 72, 69);
const auto ivory = juce::Colour::fromRGB(229, 225, 216);
const auto muted = juce::Colour::fromRGB(137, 132, 148);
const auto jade = juce::Colour::fromRGB(57, 230, 174);
const auto lagoon = juce::Colour::fromRGB(34, 171, 159);
const auto coral = juce::Colour::fromRGB(255, 121, 95);
const auto ice = juce::Colour::fromRGB(151, 230, 216);
} // namespace palette

WavoriaLookAndFeel::WavoriaLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, palette::ivory);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderFillColourId, palette::jade);
    setColour(juce::Label::textColourId, palette::ivory);
    setColour(juce::ComboBox::textColourId, palette::ivory);
    setColour(juce::ComboBox::backgroundColourId, palette::panel);
    setColour(juce::ComboBox::outlineColourId, palette::panelEdge);
    setColour(juce::ComboBox::arrowColourId, palette::jade);
    setColour(juce::PopupMenu::backgroundColourId, palette::panel);
    setColour(juce::PopupMenu::textColourId, palette::ivory);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, palette::lagoon.withAlpha(0.48f));
    setColour(juce::PopupMenu::highlightedTextColourId, palette::ivory);
}

void WavoriaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosition, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& slider)
{
    const auto side = static_cast<float>(std::min(width, height)) - 10.0f;
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height))
                      .withSizeKeepingCentre(side, side)
                      .reduced(4.0f);
    const auto angle = rotaryStartAngle + sliderPosition * (rotaryEndAngle - rotaryStartAngle);
    const auto centre = bounds.getCentre();
    const auto radius = bounds.getWidth() * 0.5f;
    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    g.setColour(accent.withAlpha(0.11f));
    g.fillEllipse(bounds.expanded(5.0f));

    juce::ColourGradient face(juce::Colour::fromRGB(44, 57, 54), bounds.getX(), bounds.getY(),
                              juce::Colour::fromRGB(14, 21, 21), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(face);
    g.fillEllipse(bounds);
    g.setColour(juce::Colour::fromRGB(77, 96, 91));
    g.drawEllipse(bounds, 1.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius + 3.5f, radius + 3.5f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour::fromRGB(37, 50, 48));
    g.strokePath(track, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, radius + 3.5f, radius + 3.5f, 0.0f,
                        rotaryStartAngle, angle, true);
    g.setColour(accent);
    g.strokePath(value, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved));

    juce::Path pointer;
    pointer.addRoundedRectangle(-1.4f, -radius + 7.0f, 2.8f, radius * 0.42f, 1.2f);
    g.setColour(palette::ivory.withAlpha(0.88f));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void WavoriaLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(), 1);
}

void WavoriaLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& background,
                                               bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto colour = background;
    if (isButtonDown)
        colour = colour.brighter(0.18f);
    else if (isMouseOverButton)
        colour = colour.brighter(0.09f);
    if (!button.isEnabled())
        colour = colour.withAlpha(0.28f);
    g.setColour(colour);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour((background == palette::coral ? palette::coral : palette::jade)
                    .withAlpha(button.isEnabled() ? 0.52f : 0.16f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void WavoriaLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                        bool, bool)
{
    g.setColour(palette::ivory.withAlpha(button.isEnabled() ? 0.92f : 0.28f));
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(5, 1),
                     juce::Justification::centred, 1);
}

void WavoriaLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                      int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                                      juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.5f, 0.5f,
                                         static_cast<float>(width) - 1.0f,
                                         static_cast<float>(height) - 1.0f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const auto arrowArea = juce::Rectangle<float>(static_cast<float>(buttonX),
                                                   static_cast<float>(buttonY),
                                                   static_cast<float>(buttonWidth),
                                                   static_cast<float>(buttonHeight));
    juce::Path arrow;
    arrow.addTriangle(arrowArea.getCentreX() - 4.0f, arrowArea.getCentreY() - 2.0f,
                      arrowArea.getCentreX() + 4.0f, arrowArea.getCentreY() - 2.0f,
                      arrowArea.getCentreX(), arrowArea.getCentreY() + 3.0f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(arrow);
}

ParameterKnob::ParameterKnob(juce::AudioProcessorValueTreeState& state,
                             const juce::String& parameterId,
                             const juce::String& displayName,
                             const juce::String& suffix)
    : name(displayName)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 17);
    slider.setTextValueSuffix(suffix);
    if (auto* parameter = state.getParameter(parameterId))
        slider.setDoubleClickReturnValue(true, parameter->convertFrom0to1(parameter->getDefaultValue()));
    slider.setColour(juce::Slider::rotarySliderFillColourId, palette::jade);
    addAndMakeVisible(slider);
    attachment = std::make_unique<Attachment>(state, parameterId, slider);
}

void ParameterKnob::paint(juce::Graphics& g)
{
    g.setColour(palette::muted);
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawFittedText(name.toUpperCase(), getLocalBounds().removeFromTop(17), juce::Justification::centred, 1);
}

void ParameterKnob::resized()
{
    slider.setBounds(getLocalBounds().withTrimmedTop(14));
}

TerrainGlobe::TerrainGlobe(WavoriaAudioProcessor& p)
    : processor(p)
{
    setOpaque(false);
    startTimerHz(30);
}

wavoria::dsp::TerrainParameters TerrainGlobe::readTerrainParameters() const noexcept
{
    const auto load = [this](const char* id)
    {
        return processor.parameters.getRawParameterValue(id)->load(std::memory_order_relaxed);
    };
    wavoria::dsp::TerrainParameters parameters;
    parameters.topology = load("topology");
    parameters.contour = load("contour");
    parameters.fold = load("fold");
    parameters.symmetry = load("symmetry");
    parameters.orbit = load("orbit");
    parameters.radius = load("radius");
    parameters.rotation = load("rotation");
    parameters.drift = load("drift");
    parameters.interaction = load("interaction");
    parameters.deform = load("deform");
    parameters.memory = load("memory");
    parameters.gravity = load("gravity");
    return parameters;
}

juce::Point<float> TerrainGlobe::project(float x, float y, float height,
                                         float rotation, float radius) const noexcept
{
    const auto area = getLocalBounds().toFloat();
    const auto centre = juce::Point<float>(area.getCentreX(), area.getCentreY() - 8.0f);
    const auto longitude = x * juce::MathConstants<float>::pi + rotation;
    const auto latitude = y * juce::MathConstants<float>::halfPi * 0.92f;
    const auto radial = radius * (1.0f + height * 0.075f);
    const auto cosLatitude = std::cos(latitude);
    return { centre.x + radial * cosLatitude * std::sin(longitude),
             centre.y - radial * std::sin(latitude) };
}

bool TerrainGlobe::isFrontFacing(float x, float y, float rotation) const noexcept
{
    const auto longitude = x * juce::MathConstants<float>::pi + rotation;
    const auto latitude = y * juce::MathConstants<float>::halfPi * 0.92f;
    return std::cos(latitude) * std::cos(longitude) >= -0.035f;
}

void TerrainGlobe::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    const auto fieldEnergy = juce::jlimit(0.0f, 1.0f, processor.getFieldEnergy() * 16.0f);
    const auto voiceCount = processor.getActiveVoiceCount();
    const auto parameters = readTerrainParameters();
    const auto radius = std::min(area.getWidth() * 0.39f, area.getHeight() * 0.39f);
    const auto centre = juce::Point<float>(area.getCentreX(), area.getCentreY() - 8.0f);
    const auto rotation = animation * (0.055f + parameters.drift * 0.095f);
    const auto sphere = juce::Rectangle<float>(centre.x - radius, centre.y - radius,
                                               radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colours::black.withAlpha(0.52f));
    g.fillEllipse(sphere.translated(0.0f, 13.0f).expanded(18.0f, 5.0f));

    juce::ColourGradient halo(palette::jade.withAlpha(0.22f + fieldEnergy * 0.18f), centre.x, centre.y,
                              palette::jade.withAlpha(0.0f), centre.x + radius * 1.38f, centre.y, true);
    g.setGradientFill(halo);
    g.fillEllipse(sphere.expanded(34.0f));

    juce::ColourGradient glass(juce::Colour::fromRGB(18, 42, 38), sphere.getX(), sphere.getY(),
                               juce::Colour::fromRGB(5, 10, 12), sphere.getRight(), sphere.getBottom(), false);
    glass.addColour(0.42, juce::Colour::fromRGB(10, 28, 27));
    g.setGradientFill(glass);
    g.fillEllipse(sphere);

    juce::Path sphereClip;
    sphereClip.addEllipse(sphere);
    g.saveState();
    g.reduceClipRegion(sphereClip, juce::AffineTransform());

    constexpr int ribbonCount = 15;
    constexpr int steps = 150;
    for (int ribbon = 0; ribbon < ribbonCount; ++ribbon)
    {
        const auto y = -0.9f + static_cast<float>(ribbon) * (1.8f / static_cast<float>(ribbonCount - 1));
        juce::Path path;
        bool drawing = false;
        for (int step = 0; step <= steps; ++step)
        {
            const auto x = -1.0f + static_cast<float>(step) * (2.0f / static_cast<float>(steps));
            const auto height = wavoria::dsp::TerrainSurface::sampleBase(x, y, parameters)
                              + processor.getVisualFieldHeight(x, y) * parameters.deform * 0.9f
                              + std::sin(animation * 0.3f + x * 5.0f + y * 3.0f) * fieldEnergy * 0.035f;
            const auto visible = isFrontFacing(x, y, rotation);
            const auto point = project(x, y, height, rotation, radius);
            if (visible && !drawing)
            {
                path.startNewSubPath(point);
                drawing = true;
            }
            else if (visible)
                path.lineTo(point);
            else
                drawing = false;
        }

        const auto blend = static_cast<float>(ribbon) / static_cast<float>(ribbonCount - 1);
        const auto colour = blend < 0.6f
                                ? palette::jade.interpolatedWith(palette::lagoon, blend * 1.65f)
                                : palette::lagoon.interpolatedWith(palette::coral, (blend - 0.6f) * 2.5f);
        g.setColour(colour.withAlpha(0.18f + fieldEnergy * 0.24f));
        g.strokePath(path, juce::PathStrokeType(1.0f + fieldEnergy * 1.15f,
                                                juce::PathStrokeType::curved));
    }

    const auto seed = static_cast<std::uint32_t>(
        processor.parameters.getRawParameterValue("seed")->load(std::memory_order_relaxed));
    for (std::uint32_t particle = 0; particle < 760u; ++particle)
    {
        const auto a = wavoria::dsp::detail::hash(seed + particle * 747796405u);
        const auto b = wavoria::dsp::detail::hash(a ^ 0x9e3779b9u);
        const auto x = static_cast<float>(a & 0xffffu) / 32767.5f - 1.0f;
        const auto y = static_cast<float>(b & 0xffffu) / 32767.5f - 1.0f;
        if (!isFrontFacing(x, y, rotation))
            continue;
        const auto height = wavoria::dsp::TerrainSurface::sampleBase(x, y, parameters)
                          + processor.getVisualFieldHeight(x, y) * parameters.deform * 0.9f;
        const auto point = project(x, y, height, rotation, radius);
        const auto bright = static_cast<float>((a >> 20u) & 0xffu) / 255.0f;
        const auto colour = bright > 0.78f ? palette::coral : palette::jade.interpolatedWith(palette::ice, bright);
        g.setColour(colour.withAlpha(0.11f + bright * 0.34f + fieldEnergy * 0.1f));
        const auto size = 0.55f + bright * 1.25f;
        g.fillEllipse(point.x - size * 0.5f, point.y - size * 0.5f, size, size);
    }

    const std::array<juce::Colour, 4> voiceColours { palette::jade, palette::coral, palette::ice, palette::lagoon };
    for (std::size_t voice = 0; voice < histories.size(); ++voice)
    {
        const auto& history = histories[voice];
        if (history.size() < 2)
            continue;

        for (std::size_t index = 1; index < history.size(); ++index)
        {
            const auto x0 = history[index - 1].x;
            const auto y0 = history[index - 1].y;
            const auto x1 = history[index].x;
            const auto y1 = history[index].y;
            if (!isFrontFacing(x0, y0, rotation) || !isFrontFacing(x1, y1, rotation))
                continue;
            const auto h0 = wavoria::dsp::TerrainSurface::sampleBase(x0, y0, parameters)
                          + processor.getVisualFieldHeight(x0, y0) * parameters.deform * 0.9f + 0.03f;
            const auto h1 = wavoria::dsp::TerrainSurface::sampleBase(x1, y1, parameters)
                          + processor.getVisualFieldHeight(x1, y1) * parameters.deform * 0.9f + 0.03f;
            const auto p0 = project(x0, y0, h0, rotation, radius);
            const auto p1 = project(x1, y1, h1, rotation, radius);
            const auto alpha = static_cast<float>(index) / static_cast<float>(history.size());
            g.setColour(voiceColours[voice % voiceColours.size()].withAlpha(alpha * 0.62f));
            g.drawLine(juce::Line<float>(p0, p1), 0.7f + alpha * 1.8f);
        }

        const auto current = history.back();
        if (isFrontFacing(current.x, current.y, rotation))
        {
            const auto height = wavoria::dsp::TerrainSurface::sampleBase(current.x, current.y, parameters)
                              + processor.getVisualFieldHeight(current.x, current.y) * parameters.deform * 0.9f + 0.05f;
            const auto point = project(current.x, current.y, height, rotation, radius);
            const auto colour = voiceColours[voice % voiceColours.size()];
            g.setColour(colour.withAlpha(0.2f));
            g.fillEllipse(point.x - 8.0f, point.y - 8.0f, 16.0f, 16.0f);
            g.setColour(colour);
            g.fillEllipse(point.x - 2.2f, point.y - 2.2f, 4.4f, 4.4f);
        }
    }
    g.restoreState();

    g.setColour(palette::ivory.withAlpha(0.18f));
    g.drawEllipse(sphere, 1.0f);
    juce::Path litRim;
    litRim.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, 0.38f, 1.92f, true);
    g.setColour(palette::coral.withAlpha(0.32f + fieldEnergy * 0.35f));
    g.strokePath(litRim, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved));

    const auto status = voiceCount == 0 ? juce::String("FIELD IDLE")
                                        : juce::String(voiceCount) + (voiceCount == 1 ? " READER" : " READERS");
    g.setColour(palette::ivory.withAlpha(0.86f));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawFittedText("SHARED TERRAIN // LIVE", getLocalBounds().removeFromTop(28),
                     juce::Justification::centred, 1);
    g.setColour(palette::coral.withAlpha(0.82f));
    g.drawFittedText(status, getLocalBounds().removeFromBottom(46), juce::Justification::centred, 1);
    g.setColour(palette::muted);
    g.setFont(juce::FontOptions(9.5f));
    g.drawFittedText("SURFACE   /   FIELD   /   PATH   /   MEMORY",
                     getLocalBounds().removeFromBottom(25), juce::Justification::centred, 1);
}

void TerrainGlobe::timerCallback()
{
    animation += 1.0f / 30.0f;
    for (int index = 0; index < WavoriaAudioProcessor::maximumVoices; ++index)
    {
        const auto voice = processor.getVisualVoice(index);
        auto& history = histories[static_cast<std::size_t>(index)];
        if (voice.active)
        {
            history.push_back({ juce::jlimit(-1.0f, 1.0f, voice.x),
                                juce::jlimit(-1.0f, 1.0f, voice.y) });
            while (history.size() > 44)
                history.pop_front();
        }
        else if (!history.empty())
            history.pop_front();
    }
    repaint();
}

WavoriaAudioProcessorEditor::WavoriaAudioProcessorEditor(WavoriaAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), presetManager(p.parameters), globe(p)
{
    setLookAndFeel(&lookAndFeel);
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(1080, 700, 1600, 1040);
    setSize(1280, 800);
    addAndMakeVisible(globe);

    presetBox.setEditableText(true);
    presetBox.setJustificationType(juce::Justification::centredLeft);
    presetBox.setTextWhenNothingSelected("SELECT OR NAME A PRESET");
    presetBox.onChange = [this] { loadSelectedPreset(); };

    const std::array<juce::Component*, 9> presetComponents {
        &presetBox, &previousPresetButton, &nextPresetButton, &discoverButton, &newFieldButton,
        &savePresetButton, &saveAsPresetButton, &renamePresetButton, &deletePresetButton
    };
    for (auto* component : presetComponents)
        addAndMakeVisible(*component);

    previousPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };
    discoverButton.onClick = [this]
    {
        const auto name = presetManager.discover();
        processor.requestNewField();
        refreshPresetList(name);
    };
    newFieldButton.onClick = [this]
    {
        presetManager.createNewFieldSeed();
        processor.requestNewField();
    };
    savePresetButton.onClick = [this]
    {
        if (presetManager.saveUserPreset(presetManager.getCurrentPresetName()))
            refreshPresetList(presetManager.getCurrentPresetName());
    };
    saveAsPresetButton.onClick = [this]
    {
        auto name = presetBox.getText().trim();
        if (name.isEmpty() || name.equalsIgnoreCase(presetManager.getCurrentPresetName()))
            name = presetManager.getCurrentPresetName() + " COPY";
        if (presetManager.saveUserPreset(name))
            refreshPresetList(presetManager.getCurrentPresetName());
    };
    renamePresetButton.onClick = [this]
    {
        if (presetManager.renameUserPreset(presetBox.getText()))
            refreshPresetList(presetManager.getCurrentPresetName());
    };
    deletePresetButton.onClick = [this]
    {
        if (presetManager.deleteUserPreset())
        {
            presetManager.loadPreset("INIT");
            processor.requestNewField();
            refreshPresetList("INIT");
        }
    };

    discoverButton.setColour(juce::TextButton::buttonColourId, palette::lagoon.withAlpha(0.68f));
    newFieldButton.setColour(juce::TextButton::buttonColourId, palette::coral.withAlpha(0.55f));
    for (auto* button : { &previousPresetButton, &nextPresetButton, &savePresetButton,
                          &saveAsPresetButton, &renamePresetButton, &deletePresetButton })
        button->setColour(juce::TextButton::buttonColourId, palette::panel.brighter(0.10f));

    addKnob(surfaceKnobs, "topology", "Topology");
    addKnob(surfaceKnobs, "contour", "Contour");
    addKnob(surfaceKnobs, "fold", "Fold");
    addKnob(surfaceKnobs, "symmetry", "Symmetry");

    addKnob(fieldKnobs, "deform", "Deform");
    addKnob(fieldKnobs, "memory", "Memory");
    addKnob(fieldKnobs, "gravity", "Gravity");
    addKnob(fieldKnobs, "seed", "Field Seed");

    addKnob(orbitKnobs, "orbit", "Orbit");
    addKnob(orbitKnobs, "radius", "Radius");
    addKnob(orbitKnobs, "rotation", "Rotation", "°");
    addKnob(orbitKnobs, "drift", "Drift");
    addKnob(orbitKnobs, "interaction", "Interaction");

    addKnob(voiceKnobs, "attack", "Attack", " s");
    addKnob(voiceKnobs, "decay", "Decay", " s");
    addKnob(voiceKnobs, "sustain", "Sustain");
    addKnob(voiceKnobs, "release", "Release", " s");

    addKnob(outputKnobs, "tone", "Tone", " Hz");
    addKnob(outputKnobs, "drive", "Drive");
    addKnob(outputKnobs, "width", "Width");
    addKnob(outputKnobs, "level", "Level", " dB");

    for (std::size_t index = 0; index < ownedKnobs.size(); ++index)
    {
        const auto colour = index % 4 == 1 ? palette::lagoon
                           : index % 4 == 2 ? palette::coral
                           : index % 4 == 3 ? palette::ice
                                            : palette::jade;
        ownedKnobs[index]->setAccent(colour);
    }

    refreshPresetList("INIT");
    resized(); // setSize() ran before the dynamically-created controls existed.
}

WavoriaAudioProcessorEditor::~WavoriaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

ParameterKnob* WavoriaAudioProcessorEditor::addKnob(std::vector<ParameterKnob*>& group,
                                                     const char* id, const char* name,
                                                     const char* suffix)
{
    auto control = std::make_unique<ParameterKnob>(processor.parameters, id, name, suffix);
    auto* pointer = control.get();
    addAndMakeVisible(*pointer);
    ownedKnobs.push_back(std::move(control));
    group.push_back(pointer);
    return pointer;
}

void WavoriaAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient background(juce::Colour::fromRGB(13, 20, 20), 0.0f, 0.0f,
                                    palette::ink, static_cast<float>(getWidth()),
                                    static_cast<float>(getHeight()), false);
    g.setGradientFill(background);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRect(0, 0, getWidth(), 114);
    g.setColour(palette::panelEdge.withAlpha(0.7f));
    g.drawHorizontalLine(113, 0.0f, static_cast<float>(getWidth()));

    g.setColour(palette::jade);
    g.fillRoundedRectangle(22.0f, 18.0f, 34.0f, 34.0f, 4.0f);
    g.setColour(palette::ink);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawFittedText("LA", 22, 18, 34, 34, juce::Justification::centred, 1);

    juce::AttributedString brand;
    brand.setJustification(juce::Justification::centredLeft);
    brand.append("LATHAM", juce::Font(juce::FontOptions(12.0f, juce::Font::plain)), palette::ivory);
    brand.append("AUDIO", juce::Font(juce::FontOptions(12.0f, juce::Font::bold)), palette::ivory);
    brand.draw(g, juce::Rectangle<float>(68.0f, 13.0f, 180.0f, 18.0f));

    g.setColour(palette::ivory);
    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.drawText("WAVORIA", 67, 29, 220, 32, juce::Justification::centredLeft);
    g.setColour(palette::jade);
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawText("DYNAMIC WAVE TERRAIN SYNTHESIS", 278, 35, 300, 20, juce::Justification::centredLeft);

    g.setColour(palette::muted);
    g.setFont(juce::FontOptions(9.5f));
    g.drawText("FIFTY YEARS OF SYNTHESIS THAT NEVER HAPPENED", getWidth() - 390, 19, 365, 16,
               juce::Justification::centredRight);
    g.setColour(palette::coral.withAlpha(0.88f));
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawText("12-VOICE SHARED FIELD", getWidth() - 260, 38, 235, 18,
               juce::Justification::centredRight);

    g.setColour(palette::muted);
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.drawText("PRESET", 22, 80, 44, 24, juce::Justification::centredLeft);

    auto content = getLocalBounds().withTrimmedTop(124).reduced(14, 10);
    const auto sideWidth = juce::jlimit(230, 282, static_cast<int>(content.getWidth() * 0.225f));
    auto left = content.removeFromLeft(sideWidth);
    content.removeFromLeft(12);
    auto right = content.removeFromRight(sideWidth);
    content.removeFromRight(12);

    const auto leftSurface = left.removeFromTop(static_cast<int>(left.getHeight() * 0.47f));
    left.removeFromTop(10);
    drawPanel(g, leftSurface, "SURFACE");
    drawPanel(g, left, "SHARED FIELD");

    const auto orbitHeight = static_cast<int>(right.getHeight() * 0.37f);
    const auto voiceHeight = static_cast<int>(right.getHeight() * 0.30f);
    auto orbit = right.removeFromTop(orbitHeight);
    right.removeFromTop(10);
    auto voice = right.removeFromTop(voiceHeight);
    right.removeFromTop(10);
    drawPanel(g, orbit, "TRAJECTORY");
    drawPanel(g, voice, "VOICE");
    drawPanel(g, right, "OUTPUT");

    g.setColour(palette::panelEdge.withAlpha(0.42f));
    g.drawRoundedRectangle(content.toFloat(), 11.0f, 1.0f);
}

void WavoriaAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                                            const juce::String& title) const
{
    g.setColour(palette::panel.withAlpha(0.84f));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    g.setColour(palette::panelEdge.withAlpha(0.72f));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 1.0f);
    g.setColour(palette::ivory.withAlpha(0.86f));
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawText(title, bounds.getX() + 12, bounds.getY() + 7, bounds.getWidth() - 24, 15,
               juce::Justification::centredLeft);
    g.setColour(palette::jade.withAlpha(0.65f));
    g.fillRect(bounds.getX() + 12, bounds.getY() + 25, 28, 1);
}

void WavoriaAudioProcessorEditor::resized()
{
    auto presetBar = getLocalBounds().withTrimmedTop(75).removeFromTop(31).reduced(72, 1);
    previousPresetButton.setBounds(presetBar.removeFromLeft(27));
    presetBar.removeFromLeft(4);
    nextPresetButton.setBounds(presetBar.removeFromLeft(27));
    presetBar.removeFromLeft(8);
    presetBox.setBounds(presetBar.removeFromLeft(juce::jlimit(220, 300, getWidth() / 5)));
    presetBar.removeFromLeft(8);
    discoverButton.setBounds(presetBar.removeFromLeft(98));
    presetBar.removeFromLeft(6);
    newFieldButton.setBounds(presetBar.removeFromLeft(88));
    presetBar.removeFromLeft(16);
    savePresetButton.setBounds(presetBar.removeFromLeft(56));
    presetBar.removeFromLeft(6);
    saveAsPresetButton.setBounds(presetBar.removeFromLeft(70));
    presetBar.removeFromLeft(6);
    renamePresetButton.setBounds(presetBar.removeFromLeft(70));
    presetBar.removeFromLeft(6);
    deletePresetButton.setBounds(presetBar.removeFromLeft(62));

    auto content = getLocalBounds().withTrimmedTop(124).reduced(14, 10);
    const auto sideWidth = juce::jlimit(230, 282, static_cast<int>(content.getWidth() * 0.225f));
    auto left = content.removeFromLeft(sideWidth);
    content.removeFromLeft(12);
    auto right = content.removeFromRight(sideWidth);
    content.removeFromRight(12);
    globe.setBounds(content.reduced(6));

    auto surface = left.removeFromTop(static_cast<int>(left.getHeight() * 0.47f));
    left.removeFromTop(10);
    layoutGrid(surface.withTrimmedTop(30).reduced(5), surfaceKnobs, 2);
    layoutGrid(left.withTrimmedTop(30).reduced(5), fieldKnobs, 2);

    const auto orbitHeight = static_cast<int>(right.getHeight() * 0.37f);
    const auto voiceHeight = static_cast<int>(right.getHeight() * 0.30f);
    auto orbit = right.removeFromTop(orbitHeight);
    right.removeFromTop(10);
    auto voice = right.removeFromTop(voiceHeight);
    right.removeFromTop(10);
    layoutGrid(orbit.withTrimmedTop(30).reduced(4), orbitKnobs, 3);
    layoutGrid(voice.withTrimmedTop(30).reduced(4), voiceKnobs, 2);
    layoutGrid(right.withTrimmedTop(30).reduced(4), outputKnobs, 2);
}

void WavoriaAudioProcessorEditor::refreshPresetList(const juce::String& selection)
{
    const juce::ScopedValueSetter<bool> guard(refreshingPresetList, true);
    presetBox.clear(juce::dontSendNotification);
    const auto names = presetManager.getPresetNames();
    int itemId = 1;
    bool addedUserHeading = false;
    for (const auto& name : names)
    {
        if (!presetManager.isFactoryPreset(name) && !addedUserHeading)
        {
            presetBox.addSeparator();
            presetBox.addSectionHeading("USER PRESETS");
            addedUserHeading = true;
        }
        presetBox.addItem(name, itemId++);
    }

    const auto selectedName = selection.isNotEmpty() ? selection : presetManager.getCurrentPresetName();
    const auto index = names.indexOf(selectedName);
    if (index >= 0)
        presetBox.setSelectedId(index + 1, juce::dontSendNotification);
    else
        presetBox.setText(selectedName, juce::dontSendNotification);
    updatePresetButtons();
}

void WavoriaAudioProcessorEditor::updatePresetButtons()
{
    const auto isUser = presetManager.isUserPreset(presetManager.getCurrentPresetName());
    renamePresetButton.setEnabled(isUser);
    deletePresetButton.setEnabled(isUser);
}

void WavoriaAudioProcessorEditor::stepPreset(int delta)
{
    const auto names = presetManager.getPresetNames();
    if (names.isEmpty())
        return;
    auto index = names.indexOf(presetManager.getCurrentPresetName());
    if (index < 0)
        index = 0;
    index = (index + delta + names.size()) % names.size();
    if (presetManager.loadPreset(names[index]))
    {
        processor.requestNewField();
        refreshPresetList(presetManager.getCurrentPresetName());
    }
}

void WavoriaAudioProcessorEditor::loadSelectedPreset()
{
    if (refreshingPresetList || presetBox.getSelectedId() <= 0)
        return;
    if (presetManager.loadPreset(presetBox.getText()))
    {
        processor.requestNewField();
        updatePresetButtons();
    }
}

void WavoriaAudioProcessorEditor::layoutGrid(juce::Rectangle<int> bounds,
                                              const std::vector<ParameterKnob*>& controls,
                                              int columns)
{
    if (controls.empty() || columns <= 0)
        return;
    const auto rows = static_cast<int>((controls.size() + static_cast<std::size_t>(columns) - 1)
                                       / static_cast<std::size_t>(columns));
    const auto cellWidth = bounds.getWidth() / columns;
    const auto cellHeight = bounds.getHeight() / std::max(1, rows);
    for (std::size_t index = 0; index < controls.size(); ++index)
    {
        const auto column = static_cast<int>(index % static_cast<std::size_t>(columns));
        const auto row = static_cast<int>(index / static_cast<std::size_t>(columns));
        controls[index]->setBounds(bounds.getX() + column * cellWidth,
                                   bounds.getY() + row * cellHeight,
                                   cellWidth, cellHeight);
    }
}
