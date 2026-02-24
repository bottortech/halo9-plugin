#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Data/H9Library.h"

class HALO9PlayerAudioProcessor : public juce::AudioProcessor
{
public:
    HALO9PlayerAudioProcessor();
    ~HALO9PlayerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HALO9 Player"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public APVTS (direct access for attachment init-list)
    juce::AudioProcessorValueTreeState apvts;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::MidiKeyboardState& getKeyboardState() { return midiKeyboardState; }
    H9Library& getLibrary() { return library; }

    static constexpr int NUM_PADS = 8;
    juce::String padSamplePaths[NUM_PADS];

private:
    juce::MidiKeyboardState midiKeyboardState;
    H9Library library;

    double currentSampleRate { 44100.0 };
    int currentBlockSize { 512 };

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HALO9PlayerAudioProcessor)
};
