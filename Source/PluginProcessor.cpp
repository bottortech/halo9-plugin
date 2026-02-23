#include "PluginProcessor.h"
#include "PluginEditor.h"

HALO9PlayerAudioProcessor::HALO9PlayerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

HALO9PlayerAudioProcessor::~HALO9PlayerAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout 
HALO9PlayerAudioProcessor::createParameterLayout() const
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "master_volume", "Master Volume", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lowpass_cutoff", "Lowpass Cutoff",
        juce::NormalisableRange<float>(100.0f, 20000.0f, 0.0f, 0.5f), 5000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "atmosphere", "Atmosphere",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "synth_level", "Synth Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    return layout;
}

void HALO9PlayerAudioProcessor::prepareToPlay(double sr, int samplesPerBlock_)
{
    sampleRate = sr;
    samplesPerBlock = samplesPerBlock_;
}

void HALO9PlayerAudioProcessor::releaseResources() {}

void HALO9PlayerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                            juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // Pass through: clean pass-through with master volume applied
    auto masterVol = apvts.getRawParameterValue("master_volume")->load();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.applyGain(ch, 0, buffer.getNumSamples(), masterVol);
    }

    // Update keyboard state
    for (const auto metadata : midiMessages)
    {
        if (auto* m = metadata.getMessage().getRawData())
            midiKeyboardState.processNextMidiEvent(metadata.getMessage());
    }
}

void HALO9PlayerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.state.createXml();
    copyXmlToBinary(*state, destData);
}

void HALO9PlayerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* HALO9PlayerAudioProcessor::createEditor()
{
    return new HALO9PlayerAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HALO9PlayerAudioProcessor();
}
