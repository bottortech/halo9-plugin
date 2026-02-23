#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// ── HALO9 colour palette ────────────────────────────────────────────────────
// Shared by the LookAndFeel and any editor code that needs theme colours.

namespace H9
{
    inline const juce::Colour bg       { 0xff0d1117 };
    inline const juce::Colour bgLight  { 0xff0f1419 };
    inline const juce::Colour panel    { 0xff161b22 };
    inline const juce::Colour border   { 0xff30363d };
    inline const juce::Colour teal     { 0xff1ef7d6 };
    inline const juce::Colour text     { 0xffe6edf3 };
    inline const juce::Colour dimText  { 0xff8b949e };
}

// ── H9LookAndFeel ───────────────────────────────────────────────────────────
// Dark theme with teal arc knobs, styled buttons, themed scrollbars,
// combo boxes, text editors, and section panels.

class H9LookAndFeel : public juce::LookAndFeel_V4
{
public:
    H9LookAndFeel();

    // Rotary slider — arc-style with radial gradient glow
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    // Button — rounded rect with hover/press glow
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override;

    // Scrollbar — slim dark track, teal thumb
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&,
                       int x, int y, int w, int h,
                       bool isScrollbarVertical, int thumbStartPosition,
                       int thumbSize, bool isMouseOver, bool isMouseDown) override;

    // Text editor — teal focus ring
    void drawTextEditorOutline(juce::Graphics&, int w, int h,
                               juce::TextEditor&) override;

    // ComboBox — dark body with teal border
    void drawComboBox(juce::Graphics&, int w, int h, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;

    int  getDefaultScrollbarWidth() override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};
