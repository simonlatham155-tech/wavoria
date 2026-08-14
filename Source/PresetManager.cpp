#include "PresetManager.h"

#include <array>

namespace
{
constexpr std::size_t parameterCount = 21;
using PresetValues = std::array<float, parameterCount>;

struct FactoryPreset
{
    const char* name;
    PresetValues values;
};

constexpr std::array<const char*, parameterCount> parameterIds {
    "topology", "contour", "fold", "symmetry",
    "orbit", "radius", "rotation", "drift", "interaction",
    "deform", "memory", "gravity",
    "attack", "decay", "sustain", "release",
    "tone", "drive", "width", "level", "seed"
};

const std::array<FactoryPreset, 8> factoryPresets {{
    { "INIT", { 0.18f, 0.28f, 0.12f, 0.52f, 0.24f, 0.62f, 0.0f, 0.08f, 0.16f,
                0.22f, 0.66f, 0.12f, 0.012f, 0.42f, 0.78f, 1.4f, 12000.0f, 0.15f, 0.72f, -12.0f, 1979.0f } },
    { "LIVING DUNE", { 0.08f, 0.22f, 0.08f, 0.72f, 0.10f, 0.68f, -18.0f, 0.14f, 0.12f,
                       0.28f, 0.72f, 0.18f, 0.35f, 1.10f, 0.82f, 3.8f, 9200.0f, 0.10f, 0.88f, -15.0f, 1979.0f } },
    { "OBSIDIAN BASS", { 0.36f, 0.48f, 0.52f, 0.85f, 0.18f, 0.32f, 22.0f, 0.02f, 0.42f,
                         0.18f, 0.25f, 0.38f, 0.002f, 0.18f, 0.72f, 0.22f, 2600.0f, 0.48f, 0.26f, -9.0f, 84.0f } },
    { "CORAL PLUCK", { 0.62f, 0.70f, 0.42f, 0.38f, 0.68f, 0.48f, -32.0f, 0.06f, 0.55f,
                       0.45f, 0.18f, 0.20f, 0.001f, 0.16f, 0.0f, 0.38f, 6800.0f, 0.28f, 0.66f, -11.0f, 311.0f } },
    { "GRAVITY BELL", { 0.48f, 0.55f, 0.15f, 0.92f, 0.74f, 0.58f, 45.0f, 0.04f, 0.68f,
                        0.33f, 0.76f, 0.82f, 0.001f, 1.60f, 0.28f, 4.8f, 14000.0f, 0.12f, 0.92f, -16.0f, 1979.0f } },
    { "MEMORY PAD", { 0.14f, 0.18f, 0.08f, 0.75f, 0.42f, 0.78f, 0.0f, 0.28f, 0.32f,
                      0.68f, 0.94f, 0.52f, 1.80f, 3.20f, 0.86f, 7.5f, 7200.0f, 0.08f, 1.0f, -18.0f, 505.0f } },
    { "CHORD WEATHER", { 0.27f, 0.32f, 0.25f, 0.44f, 0.58f, 0.83f, 19.0f, 0.56f, 0.76f,
                         0.88f, 0.98f, 0.72f, 0.28f, 2.40f, 0.74f, 6.5f, 10800.0f, 0.20f, 1.0f, -19.0f, 909.0f } },
    { "CRYSTAL ORBIT", { 0.91f, 0.66f, 0.72f, 0.32f, 0.88f, 0.72f, -50.0f, 0.16f, 0.82f,
                         0.32f, 0.58f, 0.36f, 0.01f, 0.90f, 0.68f, 2.7f, 16500.0f, 0.40f, 0.94f, -17.0f, 1229.0f } }
}};
} // namespace

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& parameterState)
    : state(parameterState)
{
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : factoryPresets)
        names.add(preset.name);

    auto files = getPresetDirectory().findChildFiles(juce::File::findFiles, false, "*.wavoria");
    juce::StringArray userNames;
    for (const auto& file : files)
        userNames.addIfNotAlreadyThere(file.getFileNameWithoutExtension(), true);
    userNames.sort(true);
    names.addArray(userNames);
    return names;
}

bool PresetManager::isFactoryPreset(const juce::String& name) const noexcept
{
    for (const auto& preset : factoryPresets)
        if (name.equalsIgnoreCase(preset.name))
            return true;
    return false;
}

bool PresetManager::isUserPreset(const juce::String& name) const
{
    return !isFactoryPreset(name) && getUserPresetFile(name).existsAsFile();
}

void PresetManager::applyValue(const char* parameterId, float plainValue)
{
    if (auto* parameter = state.getParameter(parameterId))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
        parameter->endChangeGesture();
    }
}

bool PresetManager::loadPreset(const juce::String& name)
{
    for (const auto& preset : factoryPresets)
    {
        if (!name.equalsIgnoreCase(preset.name))
            continue;

        for (std::size_t index = 0; index < parameterIds.size(); ++index)
            applyValue(parameterIds[index], preset.values[index]);
        currentPresetName = preset.name;
        return true;
    }

    const auto file = getUserPresetFile(name);
    if (!file.existsAsFile())
        return false;
    if (auto xml = juce::XmlDocument::parse(file))
    {
        auto presetState = juce::ValueTree::fromXml(*xml);
        if (presetState.isValid() && presetState.hasType(state.state.getType()))
        {
            state.replaceState(presetState);
            currentPresetName = file.getFileNameWithoutExtension();
            return true;
        }
    }
    return false;
}

bool PresetManager::saveUserPreset(const juce::String& requestedName)
{
    auto name = sanitiseName(requestedName);
    if (name.isEmpty())
        return false;
    if (isFactoryPreset(name))
        name += " Copy";

    const auto directory = getPresetDirectory();
    if (!directory.isDirectory() && !directory.createDirectory())
        return false;
    if (auto xml = state.copyState().createXml())
    {
        const auto success = xml->writeTo(getUserPresetFile(name));
        if (success)
            currentPresetName = name;
        return success;
    }
    return false;
}

bool PresetManager::renameUserPreset(const juce::String& requestedName)
{
    if (isFactoryPreset(currentPresetName))
        return false;
    const auto name = sanitiseName(requestedName);
    if (name.isEmpty() || isFactoryPreset(name))
        return false;

    const auto oldFile = getUserPresetFile(currentPresetName);
    const auto newFile = getUserPresetFile(name);
    if (!oldFile.existsAsFile() || newFile.existsAsFile())
        return false;
    if (oldFile.moveFileTo(newFile))
    {
        currentPresetName = name;
        return true;
    }
    return false;
}

bool PresetManager::deleteUserPreset()
{
    if (isFactoryPreset(currentPresetName))
        return false;
    const auto file = getUserPresetFile(currentPresetName);
    if (file.existsAsFile() && file.deleteFile())
    {
        currentPresetName = "INIT";
        return true;
    }
    return false;
}

juce::String PresetManager::discover()
{
    auto& random = juce::Random::getSystemRandom();
    const auto baseIndex = 1 + random.nextInt(static_cast<int>(factoryPresets.size()) - 1);
    const auto& base = factoryPresets[static_cast<std::size_t>(baseIndex)];

    for (std::size_t index = 0; index < parameterIds.size(); ++index)
    {
        auto* parameter = state.getParameter(parameterIds[index]);
        if (parameter == nullptr)
            continue;

        const auto id = juce::String(parameterIds[index]);
        if (id == "seed")
        {
            applyValue(parameterIds[index], static_cast<float>(1 + random.nextInt(999999)));
            continue;
        }

        auto normalised = parameter->convertTo0to1(base.values[index]);
        const auto amount = id == "attack" || id == "decay" || id == "release" || id == "tone"
                                ? 0.10f : 0.16f;
        normalised = juce::jlimit(0.0f, 1.0f,
                                  normalised + (random.nextFloat() * 2.0f - 1.0f) * amount);
        applyValue(parameterIds[index], parameter->convertFrom0to1(normalised));
    }

    // Discoveries should always audition at a safe, comparable output level.
    applyValue("level", -16.0f + random.nextFloat() * 4.0f);
    currentPresetName = "DISCOVERY " + juce::String(1000 + random.nextInt(9000));
    return currentPresetName;
}

void PresetManager::createNewFieldSeed()
{
    auto& random = juce::Random::getSystemRandom();
    applyValue("seed", static_cast<float>(1 + random.nextInt(999999)));
}

juce::File PresetManager::getPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Latham Audio")
        .getChildFile("Wavoria")
        .getChildFile("Presets");
}

juce::File PresetManager::getUserPresetFile(const juce::String& name) const
{
    return getPresetDirectory().getChildFile(sanitiseName(name) + ".wavoria");
}

juce::String PresetManager::sanitiseName(const juce::String& name)
{
    auto clean = name.trim();
    if (clean.endsWithIgnoreCase(".wavoria"))
        clean = clean.dropLastCharacters(9);
    return juce::File::createLegalFileName(clean).trim();
}
