#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>


// ── PadSampler ────────────────────────────────────────────────────────────────
//
// 8 one-shot drum pads, MIDI-mapped C1(36)–G1(43).
// Up to NUM_VOICES simultaneous playback voices (round-robin steal).
// Each pad stores its sample in memory as a float buffer.

static constexpr int NUM_PADS       = 8;
static constexpr int MIDI_PAD_START = 36;   // C1
static constexpr int NUM_VOICES     = 8;

class PadSampler
{
public:
    PadSampler();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock  (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    // Load a WAV/MP3/AIFF into pad slot [0..7]. Returns false if read fails.
    bool loadSample (int padIndex, const juce::File& file);
    void clearSample(int padIndex);

    juce::String getSampleName(int padIndex) const;
    bool         hasSample    (int padIndex) const;

    // Trigger directly from UI (no MIDI)
    void triggerPad(int padIndex, float velocity = 0.85f);

private:
    // ── Per-pad sample storage ────────────────────────────────────────────────
    struct Pad
    {
        juce::AudioBuffer<float> buffer;
        juce::String             fileName;
        bool                     hasSound { false };
    };
    Pad pads[NUM_PADS];

    // ── Voice pool ────────────────────────────────────────────────────────────
    struct Voice
    {
        const juce::AudioBuffer<float>* srcBuffer { nullptr };
        int   position { -1 };
        float gain     { 1.0f };

        bool isPlaying() const
        {
            return srcBuffer != nullptr
                && position >= 0
                && position < srcBuffer->getNumSamples();
        }
    };
    Voice voices[NUM_VOICES];
    int   nextVoice { 0 };

    juce::AudioFormatManager formatManager;
    double currentSampleRate { 44100.0 };

    void startVoice(const juce::AudioBuffer<float>& sample, float velocity);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadSampler)
};
