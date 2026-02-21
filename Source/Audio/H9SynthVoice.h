#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

// ── H9SynthSound ─────────────────────────────────────────────────────────────
// Responds to all MIDI notes EXCEPT the drum pad range (36–43).

struct H9SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote(int note) override    { return note < MIDI_PAD_START
                                                   || note > MIDI_PAD_START + NUM_PADS - 1; }
    bool appliesToChannel(int) override      { return true; }
};

// ── H9SynthVoice ─────────────────────────────────────────────────────────────
// Polyphonic saw oscillator with ADSR envelope.
// Renders into the host buffer additively.

class H9SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<H9SynthSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        level     = velocity;
        adsr.setSampleRate(getSampleRate());
        adsr.setParameters(adsrParams);
        adsr.noteOn();
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            adsr.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override
    {
        if (!adsr.isActive())
            return;

        const double sampleRate = getSampleRate();
        const double phaseInc   = frequency / sampleRate;

        for (int i = startSample; i < startSample + numSamples; ++i)
        {
            // Saw wave: linearly ramps from -1 to +1
            const float sample = (float)(2.0 * phase - 1.0) * level * adsr.getNextSample();

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, i, sample * 0.3f);   // gain tame

            phase += phaseInc;
            if (phase >= 1.0)
                phase -= 1.0;
        }

        if (!adsr.isActive())
            clearCurrentNote();
    }

    void setADSR(const juce::ADSR::Parameters& params) { adsrParams = params; }

private:
    double phase     { 0.0 };
    double frequency { 440.0 };
    float  level     { 0.0f };

    juce::ADSR            adsr;
    juce::ADSR::Parameters adsrParams { 0.05f, 0.2f, 0.7f, 0.3f };
};
