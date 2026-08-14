#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace wavoria::dsp
{
namespace detail
{
constexpr float pi = 3.14159265358979323846f;
constexpr float tau = pi * 2.0f;

[[nodiscard]] inline float clamp(float value, float low, float high) noexcept
{
    return std::clamp(value, low, high);
}

[[nodiscard]] inline float lerp(float a, float b, float amount) noexcept
{
    return a + (b - a) * amount;
}

[[nodiscard]] inline float smoothstep(float value) noexcept
{
    value = clamp(value, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

[[nodiscard]] inline std::uint32_t hash(std::uint32_t value) noexcept
{
    value ^= value >> 16u;
    value *= 0x7feb352du;
    value ^= value >> 15u;
    value *= 0x846ca68bu;
    return value ^ (value >> 16u);
}
} // namespace detail

struct TerrainParameters
{
    float topology { 0.18f };
    float contour { 0.28f };
    float fold { 0.12f };
    float symmetry { 0.52f };

    float orbit { 0.24f };
    float radius { 0.62f };
    float rotation { 0.0f };
    float drift { 0.08f };
    float interaction { 0.16f };

    float deform { 0.22f };
    float memory { 0.66f };
    float gravity { 0.12f };
    float tone { 12000.0f };
    float drive { 0.15f };

    float attack { 0.012f };
    float decay { 0.42f };
    float sustain { 0.78f };
    float release { 1.4f };
};

class TerrainSurface
{
public:
    [[nodiscard]] static float sampleBase(float x, float y, const TerrainParameters& parameters) noexcept
    {
        x = detail::clamp(x, -1.35f, 1.35f);
        y = detail::clamp(y, -1.35f, 1.35f);

        const auto core = [](float px, float py, float topology) noexcept
        {
            const auto position = detail::clamp(topology, 0.0f, 1.0f) * 3.0f;
            const auto section = std::min(2, static_cast<int>(position));
            const auto blend = detail::smoothstep(position - static_cast<float>(section));

            const auto dunes = [px, py]() noexcept
            {
                return 0.62f * std::sin(detail::pi * (1.55f * px + 0.32f * std::sin(2.1f * py)))
                     + 0.28f * std::cos(detail::pi * (1.2f * py - 0.25f * px));
            };
            const auto basins = [px, py]() noexcept
            {
                const auto r = std::sqrt(px * px + py * py);
                const auto angle = std::atan2(py, px);
                return 0.72f * std::cos(detail::pi * (3.4f * r + 0.18f * std::sin(3.0f * angle)))
                     * std::exp(-0.36f * r * r) - 0.18f * r;
            };
            const auto lattice = [px, py]() noexcept
            {
                return 0.72f * std::sin(detail::pi * 2.0f * px) * std::sin(detail::pi * 2.0f * py)
                     + 0.22f * std::cos(detail::pi * (px + py));
            };
            const auto crystal = [px, py]() noexcept
            {
                const auto r = std::sqrt(px * px + py * py);
                const auto angle = std::atan2(py, px);
                return 0.42f * std::cos(detail::pi * (3.0f * px + py))
                     + 0.36f * std::sin(detail::pi * (px - 2.0f * py))
                     + 0.18f * std::cos(6.0f * angle + 2.0f * r);
            };

            if (section == 0)
                return detail::lerp(dunes(), basins(), blend);
            if (section == 1)
                return detail::lerp(basins(), lattice(), blend);
            return detail::lerp(lattice(), crystal(), blend);
        };

        const auto raw = core(x, y, parameters.topology);
        const auto mirrored = 0.25f * (raw + core(-x, y, parameters.topology)
                                     + core(x, -y, parameters.topology)
                                     + core(-x, -y, parameters.topology));
        auto height = detail::lerp(raw, mirrored, detail::clamp(parameters.symmetry, 0.0f, 1.0f));

        const auto contour = detail::clamp(parameters.contour, 0.0f, 1.0f);
        const auto terraced = height + std::sin(height * detail::pi * 5.0f) * 0.075f;
        height = detail::lerp(height, terraced, contour);

        const auto fold = detail::clamp(parameters.fold, 0.0f, 1.0f);
        const auto drive = 1.0f + fold * 8.0f;
        const auto folded = std::tanh(height * drive) / std::tanh(drive);
        return detail::clamp(detail::lerp(height, folded, fold), -1.0f, 1.0f);
    }
};

class SharedTerrainField
{
public:
    static constexpr std::size_t gridSize = 18;

    void prepare(double newSampleRate) noexcept
    {
        sampleRate = std::max(1.0, newSampleRate);
        clear();
    }

    void clear() noexcept
    {
        heights.fill(0.0f);
        samplesUntilMaintenance = maintenanceInterval;
    }

    [[nodiscard]] float sample(float x, float y) const noexcept
    {
        const auto gx = detail::clamp((x * 0.5f + 0.5f) * static_cast<float>(gridSize - 1),
                                      0.0f, static_cast<float>(gridSize - 1));
        const auto gy = detail::clamp((y * 0.5f + 0.5f) * static_cast<float>(gridSize - 1),
                                      0.0f, static_cast<float>(gridSize - 1));
        const auto x0 = static_cast<std::size_t>(gx);
        const auto y0 = static_cast<std::size_t>(gy);
        const auto x1 = std::min(x0 + 1, gridSize - 1);
        const auto y1 = std::min(y0 + 1, gridSize - 1);
        const auto fx = gx - static_cast<float>(x0);
        const auto fy = gy - static_cast<float>(y0);

        const auto top = detail::lerp(at(x0, y0), at(x1, y0), fx);
        const auto bottom = detail::lerp(at(x0, y1), at(x1, y1), fx);
        return detail::lerp(top, bottom, fy);
    }

    [[nodiscard]] std::array<float, 2> gradient(float x, float y) const noexcept
    {
        constexpr float offset = 2.0f / static_cast<float>(gridSize - 1);
        const auto dx = (sample(x + offset, y) - sample(x - offset, y)) / (2.0f * offset);
        const auto dy = (sample(x, y + offset) - sample(x, y - offset)) / (2.0f * offset);
        return { dx, dy };
    }

    void deposit(float x, float y, float amount) noexcept
    {
        const auto gx = detail::clamp((x * 0.5f + 0.5f) * static_cast<float>(gridSize - 1),
                                      0.0f, static_cast<float>(gridSize - 1));
        const auto gy = detail::clamp((y * 0.5f + 0.5f) * static_cast<float>(gridSize - 1),
                                      0.0f, static_cast<float>(gridSize - 1));
        const auto x0 = static_cast<std::size_t>(gx);
        const auto y0 = static_cast<std::size_t>(gy);
        const auto x1 = std::min(x0 + 1, gridSize - 1);
        const auto y1 = std::min(y0 + 1, gridSize - 1);
        const auto fx = gx - static_cast<float>(x0);
        const auto fy = gy - static_cast<float>(y0);

        add(x0, y0, amount * (1.0f - fx) * (1.0f - fy));
        add(x1, y0, amount * fx * (1.0f - fy));
        add(x0, y1, amount * (1.0f - fx) * fy);
        add(x1, y1, amount * fx * fy);
    }

    void advance(float memory) noexcept
    {
        if (--samplesUntilMaintenance > 0)
            return;
        samplesUntilMaintenance = maintenanceInterval;

        const auto heldSeconds = 0.06f + std::pow(detail::clamp(memory, 0.0f, 1.0f), 3.0f) * 48.0f;
        const auto decay = std::exp(-static_cast<float>(maintenanceInterval)
                                    / (static_cast<float>(sampleRate) * heldSeconds));
        for (auto& height : heights)
            height *= decay;
    }

    [[nodiscard]] float energy() const noexcept
    {
        float sum = 0.0f;
        for (const auto height : heights)
            sum += std::abs(height);
        return sum / static_cast<float>(heights.size());
    }

    [[nodiscard]] float cellValue(std::size_t index) const noexcept
    {
        return index < heights.size() ? heights[index] : 0.0f;
    }

private:
    [[nodiscard]] float at(std::size_t x, std::size_t y) const noexcept
    {
        return heights[y * gridSize + x];
    }

    void add(std::size_t x, std::size_t y, float amount) noexcept
    {
        auto& height = heights[y * gridSize + x];
        height = detail::clamp(height + amount, -1.0f, 1.0f);
    }

    static constexpr int maintenanceInterval = 32;
    std::array<float, gridSize * gridSize> heights {};
    double sampleRate { 44100.0 };
    int samplesUntilMaintenance { maintenanceInterval };
};

class Envelope
{
public:
    void prepare(double newSampleRate) noexcept
    {
        sampleRate = std::max(1.0, newSampleRate);
        reset();
    }

    void reset() noexcept
    {
        stage = Stage::idle;
        value = 0.0f;
        releaseStart = 0.0f;
    }

    void noteOn() noexcept { stage = Stage::attack; }

    void noteOff() noexcept
    {
        if (stage != Stage::idle)
        {
            releaseStart = value;
            stage = Stage::release;
        }
    }

    [[nodiscard]] float next(const TerrainParameters& parameters) noexcept
    {
        switch (stage)
        {
            case Stage::idle:
                value = 0.0f;
                break;
            case Stage::attack:
                value += 1.0f / static_cast<float>(sampleRate * std::max(0.001f, parameters.attack));
                if (value >= 1.0f)
                {
                    value = 1.0f;
                    stage = Stage::decay;
                }
                break;
            case Stage::decay:
            {
                const auto sustain = detail::clamp(parameters.sustain, 0.0f, 1.0f);
                value -= (1.0f - sustain) / static_cast<float>(sampleRate * std::max(0.005f, parameters.decay));
                if (value <= sustain)
                {
                    value = sustain;
                    stage = Stage::sustain;
                }
                break;
            }
            case Stage::sustain:
                value = detail::clamp(parameters.sustain, 0.0f, 1.0f);
                break;
            case Stage::release:
                value -= releaseStart / static_cast<float>(sampleRate * std::max(0.005f, parameters.release));
                if (value <= 0.0f)
                    reset();
                break;
        }
        return detail::clamp(value, 0.0f, 1.0f);
    }

    [[nodiscard]] bool isActive() const noexcept { return stage != Stage::idle; }

private:
    enum class Stage { idle, attack, decay, sustain, release };
    Stage stage { Stage::idle };
    double sampleRate { 44100.0 };
    float value { 0.0f };
    float releaseStart { 0.0f };
};

struct ReaderFrame
{
    float sample { 0.0f };
    float x { 0.0f };
    float y { 0.0f };
    float imprint { 0.0f };
    bool active { false };
};

class WaveTerrainReader
{
public:
    void prepare(double newSampleRate) noexcept
    {
        sampleRate = std::max(1.0, newSampleRate);
        envelope.prepare(sampleRate);
        resetSignalPath();
    }

    void setParameters(const TerrainParameters& newParameters) noexcept { parameters = newParameters; }

    void noteOn(float newFrequency, float newVelocity, std::uint32_t seed) noexcept
    {
        resetSignalPath();
        frequency = detail::clamp(newFrequency, 1.0f, static_cast<float>(sampleRate * 0.45));
        velocity = detail::clamp(newVelocity, 0.0f, 1.0f);
        const auto hashed = detail::hash(seed == 0u ? 1u : seed);
        phase = static_cast<double>(hashed & 0xffffu) / 65536.0;
        driftPhase = static_cast<double>((hashed >> 16u) & 0xffffu) / 65536.0;
        expression = 0.0f;
        envelope.noteOn();
    }

    void noteOff() noexcept { envelope.noteOff(); }

    void setFrequency(float newFrequency) noexcept
    {
        frequency = detail::clamp(newFrequency, 1.0f, static_cast<float>(sampleRate * 0.45));
    }

    void setExpression(float amount) noexcept { expression = detail::clamp(amount, 0.0f, 1.0f); }

    [[nodiscard]] bool isActive() const noexcept { return envelope.isActive(); }

    [[nodiscard]] ReaderFrame processSample(const SharedTerrainField& field) noexcept
    {
        ReaderFrame frame;
        if (!envelope.isActive())
            return frame;

        const auto amplitude = envelope.next(parameters);
        if (!envelope.isActive())
            return frame;

        const auto theta = static_cast<float>(phase) * detail::tau;
        const auto orbitMorph = detail::clamp(parameters.orbit, 0.0f, 1.0f);
        const auto radius = 0.06f + detail::clamp(parameters.radius, 0.0f, 1.0f) * 0.9f;
        const auto rotation = parameters.rotation * detail::pi / 180.0f;
        const auto driftAmount = detail::clamp(parameters.drift, 0.0f, 1.0f);

        const auto circleX = std::cos(theta);
        const auto circleY = std::sin(theta);
        const auto lissajousX = std::sin(theta * 2.0f + 0.5f * detail::pi);
        const auto lissajousY = std::sin(theta * 3.0f);
        auto x = radius * detail::lerp(circleX, lissajousX, orbitMorph);
        auto y = radius * detail::lerp(circleY, lissajousY, orbitMorph);

        const auto rotatedX = x * std::cos(rotation) - y * std::sin(rotation);
        const auto rotatedY = x * std::sin(rotation) + y * std::cos(rotation);
        x = rotatedX + driftAmount * 0.22f * std::sin(static_cast<float>(driftPhase) * detail::tau);
        y = rotatedY + driftAmount * 0.22f * std::cos(static_cast<float>(driftPhase * 0.731) * detail::tau);

        const auto gradient = field.gradient(x, y);
        const auto gravity = detail::clamp(parameters.gravity, 0.0f, 1.0f);
        x = detail::clamp(x - gradient[0] * gravity * 0.16f, -1.2f, 1.2f);
        y = detail::clamp(y - gradient[1] * gravity * 0.16f, -1.2f, 1.2f);

        const auto fieldMix = 0.12f + 0.88f * detail::clamp(parameters.deform, 0.0f, 1.0f);
        const auto primary = TerrainSurface::sampleBase(x, y, parameters) + field.sample(x, y) * fieldMix;

        const auto satelliteTheta = theta + detail::pi * (0.38f + 0.21f * orbitMorph);
        const auto satelliteRadius = radius * (0.55f + 0.25f * std::sin(theta));
        const auto sx = satelliteRadius * std::sin(satelliteTheta * 3.0f + rotation);
        const auto sy = satelliteRadius * std::sin(satelliteTheta * 2.0f - rotation);
        const auto satellite = TerrainSurface::sampleBase(sx, sy, parameters) + field.sample(sx, sy) * fieldMix;

        const auto interaction = detail::clamp(parameters.interaction, 0.0f, 1.0f);
        auto signal = detail::lerp(primary, satellite, interaction * 0.55f);
        signal = detail::lerp(signal, primary * satellite * 1.35f, interaction * interaction * 0.72f);

        const auto cutoff = detail::clamp(parameters.tone * (0.78f + expression * 0.45f), 80.0f,
                                          static_cast<float>(sampleRate * 0.45));
        const auto coefficient = std::exp(-detail::tau * cutoff / static_cast<float>(sampleRate));
        lowpass = signal * (1.0f - coefficient) + lowpass * coefficient;

        const auto drive = 1.0f + detail::clamp(parameters.drive, 0.0f, 1.0f) * 7.0f;
        signal = std::tanh(lowpass * drive) / std::tanh(drive);

        const auto dcBlocked = signal - previousInput + 0.995f * previousOutput;
        previousInput = signal;
        previousOutput = dcBlocked;

        phase += static_cast<double>(frequency) / sampleRate;
        phase -= std::floor(phase);
        driftPhase += (0.012 + static_cast<double>(driftAmount) * 0.17) / sampleRate;
        driftPhase -= std::floor(driftPhase);

        frame.sample = detail::clamp(dcBlocked * amplitude * (0.18f + velocity * 0.82f), -1.0f, 1.0f);
        frame.x = x;
        frame.y = y;
        frame.imprint = detail::clamp(parameters.deform, 0.0f, 1.0f)
                      * (0.25f + 0.75f * velocity)
                      * (0.45f + 0.55f * expression)
                      * amplitude * std::tanh((primary + satellite) * 0.7f)
                      * (3.2f / static_cast<float>(sampleRate));
        frame.active = true;
        return frame;
    }

private:
    void resetSignalPath() noexcept
    {
        phase = 0.0;
        driftPhase = 0.0;
        lowpass = 0.0f;
        previousInput = 0.0f;
        previousOutput = 0.0f;
    }

    TerrainParameters parameters;
    Envelope envelope;
    double sampleRate { 44100.0 };
    double phase { 0.0 };
    double driftPhase { 0.0 };
    float frequency { 220.0f };
    float velocity { 0.8f };
    float expression { 0.0f };
    float lowpass { 0.0f };
    float previousInput { 0.0f };
    float previousOutput { 0.0f };
};
} // namespace wavoria::dsp
