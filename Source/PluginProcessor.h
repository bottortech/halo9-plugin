#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>

// Main audio processor (DSP + state management)
class HALO9PlayerAudioProcessor : public juce::AudioProcessor
{
public:
    HALO9PlayerAudioProcessor();
    ~HALO9PlayerAudioProcessor() override;

    // Audio processing
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Identification
    const juce::String getName() const override { return "HALO9 Player"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Default layout
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    // State
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS for parameters
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Keyboard state for UI
    juce::MidiKeyboardState& getKeyboardState() { return midiKeyboardState; }

private:
    juce::MidiKeyboardState midiKeyboardState;
    juce::AudioProcessorValueTreeState apvts;

    double sampleRate { 44100.0 };
    int samplesPerBlock { 512 };

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HALO9PlayerAudioProcessor)
};
