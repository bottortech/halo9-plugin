#include "PluginEditor.h"

HALO9PlayerAudioProcessorEditor::HALO9PlayerAudioProcessorEditor(
    HALO9PlayerAudioProcessor& p)
    : AudioProcessorEditor(&p), 
      processor(p),
      midiKeyboard(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&h9Lnf);
    setSize(800, 640);

    // Setup sliders
    auto setupSlider = [&](juce::Slider& s, juce::Label& label, const char* labelText)
    {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setPopupDisplayEnabled(true, true, this);
        s.setRange(0.0, 1.0, 0.01);
        addAndMakeVisible(&s);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, H9::dimText);
        label.setFont(juce::Font(10.0f));
        addAndMakeVisible(&label);
    };

    setupSlider(masterSlider, masterLabel, "MASTER");
    setupSlider(cutoffSlider, cutoffLabel, "CUTOFF");
    setupSlider(atmosphereSlider, atmosphereLabel, "ATMOS");
    setupSlider(synthLevelSlider, synthLevelLabel, "SYNTH");

    // Create attachments
    masterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "master_volume", masterSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "lowpass_cutoff", cutoffSlider);
    atmosphereAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "atmosphere", atmosphereSlider);
    synthLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "synth_level", synthLevelSlider);

    // Setup pad buttons
    for (int i = 0; i < NUM_PADS; ++i)
    {
        auto& btn = padButtons[i];
        btn.setButtonText("P" + juce::String(i + 1));
        btn.setColour(juce::TextButton::buttonColourId, H9::bgPanel);
        btn.setColour(juce::TextButton::textColourOffId, H9::text);
        addAndMakeVisible(&btn);
    }

    // Setup keyboard
    midiKeyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, H9::bgPanel);
    midiKeyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, H9::bgDark);
    midiKeyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, H9::dimText);
    midiKeyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, H9::accentTeal.withAlpha(0.3f));
    midiKeyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, H9::accentTeal.withAlpha(0.6f));
    midiKeyboard.setColour(juce::MidiKeyboardComponent::textLabelColourId, H9::dimText);
    addAndMakeVisible(&midiKeyboard);
}

HALO9PlayerAudioProcessorEditor::~HALO9PlayerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void HALO9PlayerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(H9::bgDark);

    // Title bar area
    g.setColour(H9::accentTeal.withAlpha(0.1f));
    g.fillRect(0, 0, getWidth(), 40);

    g.setColour(H9::text);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("HALO9 Player", 10, 5, 200, 30, juce::Justification::left);
}

void HALO9PlayerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    int padding = 10;
    int topMargin = 50;

    // Title area
    bounds.removeFromTop(topMargin);

    // Top row: 4 knobs (Master, Cutoff, Atmosphere, Synth)
    auto knobRow = bounds.removeFromTop(120);
    int knobSize = 100;
    int knobGap = 20;

    int knobX = padding;
    for (auto* label : { &masterLabel, &cutoffLabel, &atmosphereLabel, &synthLevelLabel })
    {
        auto knobArea = knobRow.removeFromLeft(knobSize + knobGap);
        label->setBounds(knobArea.getX(), knobArea.getY(), knobSize, 20);
    }

    knobX = padding;
    knobRow = bounds;
    knobRow.setHeight(120);
    for (auto* slider : { &masterSlider, &cutoffSlider, &atmosphereSlider, &synthLevelSlider })
    {
        slider->setBounds(knobX, knobRow.getY() + 20, knobSize, knobSize);
        knobX += knobSize + knobGap;
    }

    bounds.removeFromTop(120);

    // Pad buttons (2 rows of 4)
    auto padArea = bounds.removeFromTop(120);
    int padSize = 50;
    int padGap = 15;

    for (int row = 0; row < 2; ++row)
    {
        int y = padArea.getY() + row * (padSize + padGap);
        for (int col = 0; col < 4; ++col)
        {
            int x = padding + col * (padSize + padGap);
            padButtons[row * 4 + col].setBounds(x, y, padSize, padSize);
        }
    }

    bounds.removeFromTop(120);

    // MIDI keyboard at bottom
    midiKeyboard.setBounds(bounds);
}
