#pragma once
#include <juce_core/juce_core.h>

// ── FactorySounds ────────────────────────────────────────────────────────────
// Placeholder system for shipping built-in samples with HALO9.
// Currently defines the structure — actual embedded audio will be added later.

namespace FactorySounds
{
    // Sound categories
    enum class Category { Kick, Snare, HiHat, Clap, Perc, Tom, Cymbal, FX };

    // Descriptor for a factory sound
    struct SoundInfo
    {
        juce::String  name;
        Category      category;
        const char*   resourceName;   // BinaryData resource (nullptr = not yet embedded)
        int           resourceSize;
    };

    // Get the list of available factory sounds (empty until we embed audio)
    inline juce::Array<SoundInfo> getAvailableSounds()
    {
        // TODO: populate with BinaryData entries when factory WAVs are embedded
        return {};
    }

    // Check if factory sounds are available
    inline bool hasFactorySounds() { return false; }
}
