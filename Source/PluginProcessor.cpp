#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
constexpr auto midiMappingPrefix = "midi_cc_";

juce::NormalisableRange<float> skewedRange(float minimum, float maximum, float centre)
{
    juce::NormalisableRange<float> range(minimum, maximum);
    range.setSkewForCentre(centre);
    return range;
}
} // namespace

WavoriaAudioProcessor::WavoriaAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "WAVORIA_STATE", createParameterLayout())
{
    channelPitchBend.fill(0.0f);
    channelPressure.fill(0.0f);
    channelModWheel.fill(0.0f);
    sustainPedal.fill(false);
    for (std::size_t index = 0; index < midiLearnParameterIds.size(); ++index)
    {
        midiLearnParameters[index] = parameters.getParameter(midiLearnParameterIds[index]);
        midiCcAssignments[index].store(-1, std::memory_order_relaxed);
    }
}

int WavoriaAudioProcessor::midiLearnIndexFor(const juce::String& parameterId) noexcept
{
    for (std::size_t index = 0; index < midiLearnParameterIds.size(); ++index)
        if (parameterId == midiLearnParameterIds[index])
            return static_cast<int>(index);
    return -1;
}

void WavoriaAudioProcessor::beginMidiLearn(const juce::String& parameterId) noexcept
{
    midiLearnTarget.store(midiLearnIndexFor(parameterId), std::memory_order_release);
}

void WavoriaAudioProcessor::cancelMidiLearn(const juce::String& parameterId) noexcept
{
    const auto index = midiLearnIndexFor(parameterId);
    auto expected = index;
    midiLearnTarget.compare_exchange_strong(expected, -1, std::memory_order_acq_rel);
}

void WavoriaAudioProcessor::clearMidiLearn(const juce::String& parameterId) noexcept
{
    const auto index = midiLearnIndexFor(parameterId);
    if (index < 0)
        return;
    cancelMidiLearn(parameterId);
    midiCcAssignments[static_cast<std::size_t>(index)].store(-1, std::memory_order_release);
}

int WavoriaAudioProcessor::getMidiControllerForParameter(const juce::String& parameterId) const noexcept
{
    const auto index = midiLearnIndexFor(parameterId);
    return index >= 0 ? midiCcAssignments[static_cast<std::size_t>(index)].load(std::memory_order_acquire) : -1;
}

bool WavoriaAudioProcessor::isMidiLearning(const juce::String& parameterId) const noexcept
{
    return midiLearnTarget.load(std::memory_order_acquire) == midiLearnIndexFor(parameterId);
}

juce::AudioProcessorValueTreeState::ParameterLayout WavoriaAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Int = juce::AudioParameterInt;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;
    const auto id = [](const char* value) { return juce::ParameterID(value, 1); };

    layout.push_back(std::make_unique<Float>(id("topology"), "Topology", juce::NormalisableRange<float>(0.0f, 1.0f), 0.18f));
    layout.push_back(std::make_unique<Float>(id("contour"), "Contour", juce::NormalisableRange<float>(0.0f, 1.0f), 0.28f));
    layout.push_back(std::make_unique<Float>(id("fold"), "Fold", juce::NormalisableRange<float>(0.0f, 1.0f), 0.12f));
    layout.push_back(std::make_unique<Float>(id("symmetry"), "Symmetry", juce::NormalisableRange<float>(0.0f, 1.0f), 0.52f));

    layout.push_back(std::make_unique<Float>(id("orbit"), "Orbit", juce::NormalisableRange<float>(0.0f, 1.0f), 0.24f));
    layout.push_back(std::make_unique<Float>(id("radius"), "Radius", juce::NormalisableRange<float>(0.0f, 1.0f), 0.62f));
    layout.push_back(std::make_unique<Float>(id("rotation"), "Rotation", juce::NormalisableRange<float>(-180.0f, 180.0f), 0.0f));
    layout.push_back(std::make_unique<Float>(id("drift"), "Drift", juce::NormalisableRange<float>(0.0f, 1.0f), 0.08f));
    layout.push_back(std::make_unique<Float>(id("interaction"), "Interaction", juce::NormalisableRange<float>(0.0f, 1.0f), 0.16f));

    layout.push_back(std::make_unique<Float>(id("deform"), "Deform", juce::NormalisableRange<float>(0.0f, 1.0f), 0.22f));
    layout.push_back(std::make_unique<Float>(id("memory"), "Memory", juce::NormalisableRange<float>(0.0f, 1.0f), 0.66f));
    layout.push_back(std::make_unique<Float>(id("gravity"), "Gravity", juce::NormalisableRange<float>(0.0f, 1.0f), 0.12f));

    layout.push_back(std::make_unique<Float>(id("attack"), "Attack", skewedRange(0.001f, 5.0f, 0.12f), 0.012f));
    layout.push_back(std::make_unique<Float>(id("decay"), "Decay", skewedRange(0.005f, 5.0f, 0.35f), 0.42f));
    layout.push_back(std::make_unique<Float>(id("sustain"), "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.78f));
    layout.push_back(std::make_unique<Float>(id("release"), "Release", skewedRange(0.005f, 12.0f, 0.8f), 1.4f));

    layout.push_back(std::make_unique<Float>(id("tone"), "Tone", skewedRange(80.0f, 20000.0f, 3200.0f), 12000.0f));
    layout.push_back(std::make_unique<Float>(id("drive"), "Drive", juce::NormalisableRange<float>(0.0f, 1.0f), 0.15f));
    layout.push_back(std::make_unique<Float>(id("width"), "Width", juce::NormalisableRange<float>(0.0f, 1.0f), 0.72f));
    layout.push_back(std::make_unique<Float>(id("level"), "Level", juce::NormalisableRange<float>(-48.0f, 0.0f), -12.0f));
    layout.push_back(std::make_unique<Int>(id("seed"), "Field Seed", 1, 999999, 1979));
    return { layout.begin(), layout.end() };
}

void WavoriaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sharedField.prepare(sampleRate);
    for (auto& voice : voices)
    {
        voice.reader.prepare(sampleRate);
        voice.note = -1;
        voice.keyDown = false;
        voice.sustained = false;
    }

    outputGain.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 });
    outputGain.setRampDurationSeconds(0.025);
    voiceAge = 0;
    visualUpdateCountdown = 0;
}

void WavoriaAudioProcessor::releaseResources() {}

bool WavoriaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

WavoriaAudioProcessor::ParameterSnapshot WavoriaAudioProcessor::readParameters() const noexcept
{
    const auto load = [this](const char* parameterId)
    {
        return parameters.getRawParameterValue(parameterId)->load(std::memory_order_relaxed);
    };

    ParameterSnapshot snapshot;
    snapshot.terrain.topology = load("topology");
    snapshot.terrain.contour = load("contour");
    snapshot.terrain.fold = load("fold");
    snapshot.terrain.symmetry = load("symmetry");
    snapshot.terrain.orbit = load("orbit");
    snapshot.terrain.radius = load("radius");
    snapshot.terrain.rotation = load("rotation");
    snapshot.terrain.drift = load("drift");
    snapshot.terrain.interaction = load("interaction");
    snapshot.terrain.deform = load("deform");
    snapshot.terrain.memory = load("memory");
    snapshot.terrain.gravity = load("gravity");
    snapshot.terrain.attack = load("attack");
    snapshot.terrain.decay = load("decay");
    snapshot.terrain.sustain = load("sustain");
    snapshot.terrain.release = load("release");
    snapshot.terrain.tone = load("tone");
    snapshot.terrain.drive = load("drive");
    snapshot.width = load("width");
    snapshot.levelDb = load("level");
    snapshot.seed = static_cast<std::uint32_t>(load("seed"));
    return snapshot;
}

void WavoriaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    if (clearFieldRequested.exchange(false, std::memory_order_acq_rel))
        sharedField.clear();
    const auto snapshot = readParameters();
    int renderedUntil = 0;

    for (const auto metadata : midi)
    {
        const auto eventSample = juce::jlimit(0, buffer.getNumSamples(), metadata.samplePosition);
        renderRange(buffer, renderedUntil, eventSample - renderedUntil, snapshot);
        handleMidiMessage(metadata.getMessage(), snapshot);
        renderedUntil = eventSample;
    }
    renderRange(buffer, renderedUntil, buffer.getNumSamples() - renderedUntil, snapshot);

    outputGain.setGainDecibels(snapshot.levelDb);
    juce::dsp::AudioBlock<float> block(buffer);
    outputGain.process(juce::dsp::ProcessContextReplacing<float>(block));

    fieldEnergy.store(sharedField.energy(), std::memory_order_relaxed);
    for (std::size_t index = 0; index < fieldVisuals.size(); ++index)
        fieldVisuals[index].store(sharedField.cellValue(index), std::memory_order_relaxed);
    int active = 0;
    for (const auto& voice : voices)
        active += voice.reader.isActive() ? 1 : 0;
    activeVoiceCount.store(active, std::memory_order_relaxed);
}

void WavoriaAudioProcessor::renderRange(juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                                        const ParameterSnapshot& snapshot) noexcept
{
    if (numSamples <= 0)
        return;

    for (auto& voice : voices)
        voice.reader.setParameters(snapshot.terrain);

    std::array<wavoria::dsp::ReaderFrame, maximumVoices> frames;
    for (int offset = 0; offset < numSamples; ++offset)
    {
        float left = 0.0f;
        float right = 0.0f;
        int sounding = 0;

        for (std::size_t index = 0; index < voices.size(); ++index)
        {
            auto& voice = voices[index];
            if (!voice.reader.isActive())
            {
                frames[index] = {};
                continue;
            }

            const auto channelIndex = static_cast<std::size_t>(juce::jlimit(1, 16, voice.channel) - 1);
            const auto expression = std::max({ voice.notePressure,
                                               channelPressure[channelIndex],
                                               channelModWheel[channelIndex] * 0.72f });
            voice.reader.setExpression(expression);
            frames[index] = voice.reader.processSample(sharedField);
            if (!frames[index].active)
                continue;

            const auto gains = wavoria::dsp::detail::stereoGains(voice.pan, snapshot.width);
            left += frames[index].sample * gains[0];
            right += frames[index].sample * gains[1];
            ++sounding;
        }

        for (const auto& frame : frames)
            if (frame.active)
                sharedField.deposit(frame.x, frame.y, frame.imprint);
        sharedField.advance(snapshot.terrain.memory);

        const auto normalization = sounding > 0 ? 0.48f / std::sqrt(static_cast<float>(sounding)) : 0.0f;
        const auto sampleIndex = startSample + offset;
        if (buffer.getNumChannels() == 1)
            buffer.setSample(0, sampleIndex, (left + right) * 0.7071f * normalization);
        else
        {
            buffer.setSample(0, sampleIndex, left * normalization);
            buffer.setSample(1, sampleIndex, right * normalization);
        }

        if (--visualUpdateCountdown <= 0)
        {
            visualUpdateCountdown = 64;
            for (std::size_t index = 0; index < frames.size(); ++index)
            {
                visuals[index].x.store(frames[index].x, std::memory_order_relaxed);
                visuals[index].y.store(frames[index].y, std::memory_order_relaxed);
                visuals[index].energy.store(std::abs(frames[index].sample), std::memory_order_relaxed);
                visuals[index].active.store(frames[index].active, std::memory_order_relaxed);
            }
        }
    }
}

void WavoriaAudioProcessor::handleMidiMessage(const juce::MidiMessage& message,
                                               const ParameterSnapshot& snapshot) noexcept
{
    const auto channel = juce::jlimit(1, 16, message.getChannel());
    const auto channelIndex = static_cast<std::size_t>(channel - 1);

    if (message.isNoteOn())
        startNote(message.getNoteNumber(), channel, message.getFloatVelocity(), snapshot);
    else if (message.isNoteOff())
        stopNote(message.getNoteNumber(), channel, true);
    else if (message.isPitchWheel())
    {
        channelPitchBend[channelIndex] = (static_cast<float>(message.getPitchWheelValue()) - 8192.0f) / 8192.0f;
        updateChannelPitch(channel);
    }
    else if (message.isChannelPressure())
        channelPressure[channelIndex] = static_cast<float>(message.getChannelPressureValue()) / 127.0f;
    else if (message.isAftertouch())
    {
        for (auto& voice : voices)
            if (voice.channel == channel && voice.note == message.getNoteNumber())
                voice.notePressure = static_cast<float>(message.getAfterTouchValue()) / 127.0f;
    }
    else if (message.isController())
    {
        const auto controller = message.getControllerNumber();
        const auto value = message.getControllerValue();
        handleMidiControllerLearn(controller, value);
        if (controller == 1)
            channelModWheel[channelIndex] = static_cast<float>(value) / 127.0f;
        else if (controller == 64)
        {
            const auto wasDown = sustainPedal[channelIndex];
            sustainPedal[channelIndex] = value >= 64;
            if (wasDown && !sustainPedal[channelIndex])
                releaseSustainedVoices(channel);
        }
        else if (controller == 123 || controller == 120)
        {
            for (auto& voice : voices)
            {
                if (voice.channel == channel)
                {
                    voice.keyDown = false;
                    voice.sustained = false;
                    voice.reader.noteOff();
                }
            }
        }
    }
}

void WavoriaAudioProcessor::handleMidiControllerLearn(int controller, int value) noexcept
{
    if (controller < 0 || controller >= 120)
        return;

    const auto target = midiLearnTarget.exchange(-1, std::memory_order_acq_rel);
    if (target >= 0 && target < static_cast<int>(midiCcAssignments.size()))
    {
        // One CC controls one Wavoria parameter. Re-learning transfers ownership.
        for (auto& assignment : midiCcAssignments)
            if (assignment.load(std::memory_order_relaxed) == controller)
                assignment.store(-1, std::memory_order_relaxed);
        midiCcAssignments[static_cast<std::size_t>(target)].store(controller, std::memory_order_release);
    }

    const auto normalised = static_cast<float>(juce::jlimit(0, 127, value)) / 127.0f;
    for (std::size_t index = 0; index < midiCcAssignments.size(); ++index)
    {
        if (midiCcAssignments[index].load(std::memory_order_acquire) == controller)
            if (auto* parameter = midiLearnParameters[index])
                parameter->setValueNotifyingHost(normalised);
    }
}

void WavoriaAudioProcessor::startNote(int note, int channel, float velocity,
                                      const ParameterSnapshot& snapshot) noexcept
{
    auto& voice = findVoiceToUse();
    voice.note = note;
    voice.channel = channel;
    voice.age = ++voiceAge;
    voice.keyDown = true;
    voice.sustained = false;
    voice.notePressure = 0.0f;

    const auto panHash = wavoria::dsp::detail::hash(snapshot.seed
                                                    ^ static_cast<std::uint32_t>(note * 131)
                                                    ^ static_cast<std::uint32_t>(voice.age));
    voice.pan = static_cast<float>(panHash & 0xffffu) / 32767.5f - 1.0f;
    voice.reader.setParameters(snapshot.terrain);
    voice.reader.noteOn(frequencyFor(voice), velocity,
                        snapshot.seed ^ static_cast<std::uint32_t>(note * 65537) ^ static_cast<std::uint32_t>(voice.age));
}

void WavoriaAudioProcessor::stopNote(int note, int channel, bool allowTail) noexcept
{
    const auto channelIndex = static_cast<std::size_t>(juce::jlimit(1, 16, channel) - 1);
    for (auto& voice : voices)
    {
        if (voice.note != note || voice.channel != channel || !voice.keyDown)
            continue;

        voice.keyDown = false;
        if (sustainPedal[channelIndex] && allowTail)
            voice.sustained = true;
        else
        {
            voice.sustained = false;
            voice.reader.noteOff();
        }
    }
}

void WavoriaAudioProcessor::releaseSustainedVoices(int channel) noexcept
{
    for (auto& voice : voices)
    {
        if (voice.channel == channel && voice.sustained && !voice.keyDown)
        {
            voice.sustained = false;
            voice.reader.noteOff();
        }
    }
}

void WavoriaAudioProcessor::updateChannelPitch(int channel) noexcept
{
    for (auto& voice : voices)
        if (voice.channel == channel && voice.reader.isActive())
            voice.reader.setFrequency(frequencyFor(voice));
}

WavoriaAudioProcessor::VoiceSlot& WavoriaAudioProcessor::findVoiceToUse() noexcept
{
    for (auto& voice : voices)
        if (!voice.reader.isActive())
            return voice;

    return *std::min_element(voices.begin(), voices.end(), [](const auto& a, const auto& b)
    {
        if (a.keyDown != b.keyDown)
            return !a.keyDown;
        return a.age < b.age;
    });
}

float WavoriaAudioProcessor::frequencyFor(const VoiceSlot& voice) const noexcept
{
    const auto channelIndex = static_cast<std::size_t>(juce::jlimit(1, 16, voice.channel) - 1);
    const auto semitones = static_cast<float>(voice.note) + channelPitchBend[channelIndex] * 2.0f;
    return 440.0f * std::pow(2.0f, (semitones - 69.0f) / 12.0f);
}

WavoriaAudioProcessor::VisualVoice WavoriaAudioProcessor::getVisualVoice(int index) const noexcept
{
    if (index < 0 || index >= maximumVoices)
        return {};
    const auto& voice = visuals[static_cast<std::size_t>(index)];
    return { voice.x.load(std::memory_order_relaxed),
             voice.y.load(std::memory_order_relaxed),
             voice.energy.load(std::memory_order_relaxed),
             voice.active.load(std::memory_order_relaxed) };
}

float WavoriaAudioProcessor::getVisualFieldHeight(float x, float y) const noexcept
{
    constexpr auto size = wavoria::dsp::SharedTerrainField::gridSize;
    const auto gx = juce::jlimit(0.0f, static_cast<float>(size - 1),
                                 (x * 0.5f + 0.5f) * static_cast<float>(size - 1));
    const auto gy = juce::jlimit(0.0f, static_cast<float>(size - 1),
                                 (y * 0.5f + 0.5f) * static_cast<float>(size - 1));
    const auto x0 = static_cast<std::size_t>(gx);
    const auto y0 = static_cast<std::size_t>(gy);
    const auto x1 = std::min(x0 + 1, size - 1);
    const auto y1 = std::min(y0 + 1, size - 1);
    const auto fx = gx - static_cast<float>(x0);
    const auto fy = gy - static_cast<float>(y0);
    const auto at = [this](std::size_t px, std::size_t py)
    {
        return fieldVisuals[py * wavoria::dsp::SharedTerrainField::gridSize + px]
            .load(std::memory_order_relaxed);
    };
    const auto top = at(x0, y0) + (at(x1, y0) - at(x0, y0)) * fx;
    const auto bottom = at(x0, y1) + (at(x1, y1) - at(x0, y1)) * fx;
    return top + (bottom - top) * fy;
}

juce::AudioProcessorEditor* WavoriaAudioProcessor::createEditor()
{
    return new WavoriaAudioProcessorEditor(*this);
}

void WavoriaAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    for (std::size_t index = 0; index < midiLearnParameterIds.size(); ++index)
        state.setProperty(juce::String(midiMappingPrefix) + midiLearnParameterIds[index],
                          midiCcAssignments[index].load(std::memory_order_acquire), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void WavoriaAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
    {
        auto restoredState = juce::ValueTree::fromXml(*xml);
        if (!restoredState.isValid())
            return;
        for (std::size_t index = 0; index < midiLearnParameterIds.size(); ++index)
        {
            const auto property = juce::Identifier(juce::String(midiMappingPrefix)
                                                    + midiLearnParameterIds[index]);
            const auto controller = juce::jlimit(-1, 119,
                                                  static_cast<int>(restoredState.getProperty(property, -1)));
            midiCcAssignments[index].store(controller, std::memory_order_release);
            restoredState.removeProperty(property, nullptr);
        }
        midiLearnTarget.store(-1, std::memory_order_release);
        parameters.replaceState(restoredState);
        requestNewField();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavoriaAudioProcessor();
}
