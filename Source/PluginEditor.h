#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "UI/H9LookAndFeel.h"

class HALO9PlayerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit HALO9PlayerAudioProcessorEditor(HALO9PlayerAudioProcessor&);
    ~HALO9PlayerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HALO9PlayerAudioProcessor& processor;
    H9LookAndFeel h9Lnf;

    // Main parameter sliders
    juce::Slider masterSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
    juce::Slider cutoffSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
    juce::Slider atmosphereSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
    juce::Slider synthLevelSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };

    juce::Label masterLabel;
    juce::Label cutoffLabel;
    juce::Label atmosphereLabel;
    juce::Label synthLevelLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> atmosphereAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> synthLevelAttachment;

    // Keyboard
    juce::MidiKeyboardComponent midiKeyboard;

    // Pads (8 sample pads like original)
    static constexpr int NUM_PADS = 8;
    juce::TextButton padButtons[NUM_PADS];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HALO9PlayerAudioProcessorEditor)
};
