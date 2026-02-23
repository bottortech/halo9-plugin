#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

// HALO9 color/style constants
namespace H9
{
    const auto bgDark = juce::Colour(0xff0a0e14);
    const auto bgPanel = juce::Colour(0xff141820);
    const auto accentTeal = juce::Colour(0xff2ee6c9);
    const auto text = juce::Colour(0xffe0e0e0);
    const auto dimText = juce::Colour(0xff888888);
}

// HALO9 custom LookAndFeel
class H9LookAndFeel : public juce::LookAndFeel_V4
{
public:
    H9LookAndFeel();
    ~H9LookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                       bool isMouseOverButton, bool isButtonDown) override;
};
