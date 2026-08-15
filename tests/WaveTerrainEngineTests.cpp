#include "dsp/WaveTerrainEngine.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

void terrainIsBounded()
{
    wavoria::dsp::TerrainParameters parameters;
    for (int topology = 0; topology <= 10; ++topology)
    {
        parameters.topology = static_cast<float>(topology) / 10.0f;
        parameters.contour = 1.0f;
        parameters.fold = 1.0f;
        for (int y = -20; y <= 20; ++y)
        {
            for (int x = -20; x <= 20; ++x)
            {
                const auto value = wavoria::dsp::TerrainSurface::sampleBase(
                    static_cast<float>(x) / 16.0f, static_cast<float>(y) / 16.0f, parameters);
                require(std::isfinite(value), "terrain must remain finite");
                require(std::abs(value) <= 1.0001f, "terrain must remain bounded");
            }
        }
    }
}

void fieldHasMemory()
{
    wavoria::dsp::SharedTerrainField field;
    field.prepare(48000.0);
    require(field.energy() == 0.0f, "new field must be neutral");
    field.deposit(0.0f, 0.0f, 0.8f);
    const auto initial = field.energy();
    require(initial > 0.0f, "a note imprint must alter the shared field");
    for (int sample = 0; sample < 48000; ++sample)
        field.advance(0.0f);
    require(field.energy() < initial * 0.01f, "minimum memory must decay quickly");
}

void readersAreDeterministic()
{
    wavoria::dsp::SharedTerrainField fieldA;
    wavoria::dsp::SharedTerrainField fieldB;
    wavoria::dsp::WaveTerrainReader readerA;
    wavoria::dsp::WaveTerrainReader readerB;
    wavoria::dsp::TerrainParameters parameters;
    parameters.deform = 0.0f;
    fieldA.prepare(48000.0);
    fieldB.prepare(48000.0);
    readerA.prepare(48000.0);
    readerB.prepare(48000.0);
    readerA.setParameters(parameters);
    readerB.setParameters(parameters);
    readerA.noteOn(220.0f, 0.8f, 42u);
    readerB.noteOn(220.0f, 0.8f, 42u);

    for (int sample = 0; sample < 4096; ++sample)
    {
        const auto a = readerA.processSample(fieldA).sample;
        const auto b = readerB.processSample(fieldB).sample;
        require(a == b, "seeded readers must be exactly repeatable");
    }
}

void chordsReshapeTheField()
{
    wavoria::dsp::SharedTerrainField field;
    wavoria::dsp::WaveTerrainReader root;
    wavoria::dsp::WaveTerrainReader fifth;
    wavoria::dsp::TerrainParameters parameters;
    parameters.attack = 0.001f;
    parameters.deform = 1.0f;
    parameters.memory = 1.0f;
    field.prepare(48000.0);
    root.prepare(48000.0);
    fifth.prepare(48000.0);
    root.setParameters(parameters);
    fifth.setParameters(parameters);
    root.noteOn(220.0f, 1.0f, 1u);
    fifth.noteOn(329.6276f, 0.9f, 2u);

    for (int sample = 0; sample < 24000; ++sample)
    {
        const auto a = root.processSample(field);
        const auto b = fifth.processSample(field);
        field.deposit(a.x, a.y, a.imprint);
        field.deposit(b.x, b.y, b.imprint);
        field.advance(parameters.memory);
    }
    require(field.energy() > 0.0001f, "polyphony must write shared terrain geometry");
}

void releaseEventuallyFinishes()
{
    wavoria::dsp::SharedTerrainField field;
    wavoria::dsp::WaveTerrainReader reader;
    wavoria::dsp::TerrainParameters parameters;
    parameters.attack = 0.001f;
    parameters.release = 0.01f;
    field.prepare(48000.0);
    reader.prepare(48000.0);
    reader.setParameters(parameters);
    reader.noteOn(110.0f, 1.0f, 7u);
    for (int sample = 0; sample < 512; ++sample)
        static_cast<void>(reader.processSample(field));
    reader.noteOff();
    for (int sample = 0; sample < 1024; ++sample)
        static_cast<void>(reader.processSample(field));
    require(!reader.isActive(), "a released reader must become available for reuse");
}

void stereoFieldKeepsBothChannelsAlive()
{
    const auto centre = wavoria::dsp::detail::stereoGains(1.0f, 0.0f);
    require(std::abs(centre[0] - centre[1]) < 0.00001f,
            "zero width must produce a centred mono image");

    for (int step = 0; step <= 40; ++step)
    {
        const auto pan = -1.0f + static_cast<float>(step) * 0.05f;
        const auto gains = wavoria::dsp::detail::stereoGains(pan, 1.0f);
        require(gains[0] > 0.3f && gains[1] > 0.3f,
                "maximum width must retain every voice in both channels");
        require(std::abs(gains[0] * gains[0] + gains[1] * gains[1] - 1.0f) < 0.00001f,
                "stereo panning must preserve constant power");
    }
}
} // namespace

int main()
{
    terrainIsBounded();
    fieldHasMemory();
    readersAreDeterministic();
    chordsReshapeTheField();
    releaseEventuallyFinishes();
    stereoFieldKeepsBothChannelsAlive();
    std::cout << "Wavoria DSP tests passed\n";
    return 0;
}
