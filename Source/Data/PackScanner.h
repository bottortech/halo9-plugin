#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>


// ── PackScanner ────────────────────────────────────────────────────────────────
//
// Scans the user's HALO9 Packs directory:
//   Windows : %USERPROFILE%\Documents\HALO9\Packs
//   macOS   : ~/Documents/HALO9/Packs
//
// Expected pack layout:
//   PackName/
//     Drums/   ← one-shot WAV files, assigned to pads
//     Loops/   ← loop WAV files, loaded into the loop slot
//
// Any audio file directly in PackName/ is treated as a drum sample.

struct Pack
{
    juce::String         name;
    juce::File           directory;
    juce::Array<juce::File> drumFiles;
    juce::Array<juce::File> loopFiles;
};

class PackScanner
{
public:
    PackScanner() = default;

    /** Rescans the packs directory. Call once at startup and on refresh. */
    void scan();

    const juce::Array<Pack>& getPacks() const { return packs; }

    /** Returns ~/Documents/HALO9/Packs, creating it if needed. */
    static juce::File getPacksDirectory();

private:
    juce::Array<Pack> packs;

    static juce::Array<juce::File> scanAudioFiles(const juce::File& dir);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PackScanner)
};
