#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>


// ── LoopPlayer ─────────────────────────────────────────────────────────────────
//
// Single WAV loop slot: load file, play/stop, looping enabled.
// Mixes output into the caller's AudioBuffer (additive).

class LoopPlayer
{
public:
    LoopPlayer();
    ~LoopPlayer();

    void prepareToPlay  (double sampleRate, int samplesPerBlock);
    void processBlock   (juce::AudioBuffer<float>& outputBuffer);
    void releaseResources();

    // Thread-safe from message thread (UI callbacks)
    bool loadFile (const juce::File& file);
    void play     ();
    void stop     ();
    bool isPlaying() const;

    void  setVolume (float vol);
    float getVolume () const { return volume.load(); }

    juce::String getFileName() const { return fileName; }
    bool         hasFile    () const { return fileLoaded; }

private:
    juce::AudioFormatManager  formatManager;
    juce::TimeSliceThread     loadThread { "LoopPlayerLoader" };

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource                     transport;

    // Pre-allocated mixing buffer (avoid per-block allocation)
    juce::AudioBuffer<float> tempBuffer;

    juce::String fileName;
    bool         fileLoaded    { false };
    std::atomic<float> volume  { 0.8f };

    double preparedSampleRate { 44100.0 };
    int    preparedBlockSize  { 512 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopPlayer)
};
