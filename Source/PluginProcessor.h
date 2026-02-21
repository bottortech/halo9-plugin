#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Audio/PadSampler.h"
#include "Audio/H9SynthVoice.h"
#include "Data/H9Library.h"

// ── HALO9PlayerAudioProcessor ──────────────────────────────────────────────────
//
// Parameter IDs:
//   "master_volume"  0.0 – 1.0   (default 0.7)
//   "lowpass_cutoff" 100 – 20000 Hz (default 20000, skew towards low end)
//   "atmosphere"     0.0 – 1.0   (default 0.0 = dry)
//   "synth_level"    0.0 – 1.0   (default 0.6)
//
// Signal chain:
//   [Synthesiser] ─┐
//                   ├─ mix ─► [LPF] ─► [Reverb] ─► [Stereo Width] ─► [Master Gain]
//   [PadSampler]  ─┘

class HALO9PlayerAudioProcessor : public juce::AudioProcessor
{
public:
    HALO9PlayerAudioProcessor();
    ~HALO9PlayerAudioProcessor() override;

    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock   (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi()    const override { return true; }
    bool producesMidi()   const override { return false; }
    bool isMidiEffect()   const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int  getNumPrograms()  override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ── Public state ──────────────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState apvts;

    PadSampler&              getPadSampler()     { return padSampler; }
    juce::Synthesiser&       getSynth()          { return synth; }
    juce::MidiKeyboardState& getKeyboardState()  { return keyboardState; }
    H9Library&               getLibrary()        { return library; }

    juce::String padSamplePaths[NUM_PADS];
    juce::String currentPackName;

private:
    PadSampler              padSampler;
    juce::Synthesiser       synth;
    juce::MidiKeyboardState keyboardState;
    H9Library               library;

    // ── DSP chain (LPF → Reverb → Master Gain) ───────────────────────────────
    using IIRCoeffs  = juce::dsp::IIR::Coefficients<float>;
    using IIRFilter  = juce::dsp::IIR::Filter<float>;
    using IIRDup     = juce::dsp::ProcessorDuplicator<IIRFilter, IIRCoeffs>;
    using Chain      = juce::dsp::ProcessorChain<IIRDup, juce::dsp::Reverb, juce::dsp::Gain<float>>;
    Chain dspChain;

    double currentSampleRate { 44100.0 };

    void updateDSP();
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HALO9PlayerAudioProcessor)
};
