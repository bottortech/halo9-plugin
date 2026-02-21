#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

// ── Parameter layout ──────────────────────────────────────────────────────────

juce::AudioProcessorValueTreeState::ParameterLayout
HALO9PlayerAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_volume", 1), "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lowpass_cutoff", 1), "Lowpass Cutoff",
        juce::NormalisableRange<float>(100.0f, 20000.0f, 0.0f, 0.3f), 20000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("atmosphere", 1), "Atmosphere",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("synth_level", 1), "Synth Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.6f));

    return { params.begin(), params.end() };
}

// ── Constructor / Destructor ──────────────────────────────────────────────────

HALO9PlayerAudioProcessor::HALO9PlayerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "HALO9State", createLayout())
{
    std::cout << "HALO9 Player: AudioProcessor constructed\n";
    std::cout.flush();
    // ── Polyphonic synth: 1 sound + 8 voices ──────────────────────────────
    synth.addSound(new H9SynthSound());

    for (int i = 0; i < 8; ++i)
        synth.addVoice(new H9SynthVoice());

    // ── Load library ────────────────────────────────────────────────────
    auto libRoot = H9Library::findLibraryRoot();
    if (libRoot != juce::File())
        library.loadFromDirectory(libRoot);
}

HALO9PlayerAudioProcessor::~HALO9PlayerAudioProcessor() {}

// ── Bus layout ────────────────────────────────────────────────────────────────

bool HALO9PlayerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void HALO9PlayerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    synth.setCurrentPlaybackSampleRate(sampleRate);
    padSampler.prepareToPlay(sampleRate, samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels      = 2;

    dspChain.prepare(spec);
    dspChain.reset();
    updateDSP();
}

void HALO9PlayerAudioProcessor::releaseResources()
{
    dspChain.reset();
}

// ── processBlock ──────────────────────────────────────────────────────────────

void HALO9PlayerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalChannels = getTotalNumOutputChannels();
    for (int i = getTotalNumInputChannels(); i < totalChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // ── Inject on-screen keyboard MIDI into the buffer ─────────────────────
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // ── Render synth (additive into buffer) ────────────────────────────────
    const float synthLevel = apvts.getRawParameterValue("synth_level")->load();

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply synth level scaling
    if (synthLevel < 0.99f)
    {
        // Scale only synth contribution — but since pad hasn't rendered yet,
        // the buffer IS the synth output at this point.
        buffer.applyGain(synthLevel);
    }

    // ── Render pad sampler (additive into buffer) ──────────────────────────
    padSampler.processBlock(buffer, midiMessages);

    // ── Stereo width via M/S (atmosphere drives width 1 → 2.2) ────────────
    if (buffer.getNumChannels() >= 2)
    {
        const float atm   = apvts.getRawParameterValue("atmosphere")->load();
        const float width = 1.0f + atm * 1.2f;
        const float mid   = 0.5f * (1.0f + 1.0f / width);
        const float side  = 0.5f * (1.0f - 1.0f / width);

        auto* L = buffer.getWritePointer(0);
        auto* R = buffer.getWritePointer(1);

        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            const float m = L[n] + R[n];
            const float s = L[n] - R[n];
            L[n] = m * mid + s * side;
            R[n] = m * mid - s * side;
        }
    }

    // ── DSP chain: LPF → Reverb → Master Gain ─────────────────────────────
    updateDSP();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    dspChain.process(ctx);
}

// ── DSP parameter sync ────────────────────────────────────────────────────────

void HALO9PlayerAudioProcessor::updateDSP()
{
    // ── LPF (index 0) ─────────────────────────────────────────────────────
    float cutoff  = apvts.getRawParameterValue("lowpass_cutoff")->load();
    const float atm = apvts.getRawParameterValue("atmosphere")->load();

    cutoff *= (1.0f - atm * 0.5f);
    cutoff  = juce::jlimit(100.0f, 20000.0f, cutoff);

    *dspChain.get<0>().state =
        *IIRCoeffs::makeLowPass(currentSampleRate, static_cast<double>(cutoff), 0.707);

    // ── Reverb (index 1) ──────────────────────────────────────────────────
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize   = atm * 0.85f;
    reverbParams.wetLevel   = atm * 0.45f;
    reverbParams.dryLevel   = 1.0f - atm * 0.25f;
    reverbParams.damping    = 0.5f + atm * 0.3f;
    reverbParams.width      = 0.5f + atm * 0.5f;
    reverbParams.freezeMode = 0.0f;
    dspChain.get<1>().setParameters(reverbParams);

    // ── Master Gain (index 2) ─────────────────────────────────────────────
    const float masterVol = apvts.getRawParameterValue("master_volume")->load();
    dspChain.get<2>().setGainLinear(masterVol);
}

// ── State ─────────────────────────────────────────────────────────────────────

void HALO9PlayerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    auto padsElem = state.getOrCreateChildWithName("PadPaths", nullptr);
    for (int i = 0; i < NUM_PADS; ++i)
        padsElem.setProperty("pad" + juce::String(i), padSamplePaths[i], nullptr);

    state.setProperty("currentPackName", currentPackName, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HALO9PlayerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (!xml) return;

    auto state = juce::ValueTree::fromXml(*xml);
    if (!state.isValid()) return;

    apvts.replaceState(state);

    auto padsElem = state.getChildWithName("PadPaths");
    if (padsElem.isValid())
    {
        for (int i = 0; i < NUM_PADS; ++i)
        {
            padSamplePaths[i] = padsElem.getProperty("pad" + juce::String(i)).toString();

            if (padSamplePaths[i].isNotEmpty())
            {
                juce::File f(padSamplePaths[i]);
                if (f.existsAsFile())
                    padSampler.loadSample(i, f);
            }
        }
    }

    currentPackName = state.getProperty("currentPackName").toString();
}

// ── Editor ────────────────────────────────────────────────────────────────────

juce::AudioProcessorEditor* HALO9PlayerAudioProcessor::createEditor()
{
    return new HALO9PlayerAudioProcessorEditor(*this);
}

// ── Plugin factory ────────────────────────────────────────────────────────────

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HALO9PlayerAudioProcessor();
}
