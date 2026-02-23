#include "H9LookAndFeel.h"

H9LookAndFeel::H9LookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, H9::bgDark);
    setColour(juce::Label::textColourId, H9::text);
    setColour(juce::Slider::rotarySliderFillColourId, H9::accentTeal);
    setColour(juce::Slider::rotarySliderOutlineColourId, H9::dimText);
    setColour(juce::TextButton::buttonColourId, H9::bgPanel);
    setColour(juce::TextButton::textColourOffId, H9::text);
    setColour(juce::TextButton::textColourOnId, H9::accentTeal);
}

void H9LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                     float sliderPosProportional, float rotaryStartAngle,
                                     float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 2.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Fill background circle
    g.setColour(H9::bgPanel);
    g.fillEllipse(rx, ry, rw, rw);

    // Draw knob arc
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                           rotaryStartAngle, angle, true);
    g.strokePath(valueArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw outline
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.drawEllipse(rx, ry, rw, rw, 1.5f);

    // Draw pointer
    g.setColour(H9::accentTeal);
    juce::Path pointer;
    auto pointerLength = radius * 0.33f;
    auto pointerAngle = angle - juce::MathConstants<float>::halfPi;
    pointer.addTriangle(
        centreX,
        centreY - pointerLength / 2.0f,
        centreX + 2.0f,
        centreY + pointerLength,
        centreX - 2.0f,
        centreY + pointerLength
    );
    g.fillPath(pointer, juce::AffineTransform::rotation(pointerAngle, centreX, centreY));
}

void H9LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                         const juce::Colour& backgroundColour,
                                         bool isMouseOverButton, bool isButtonDown)
{
    auto r = b.getLocalBounds().toFloat().reduced(2.0f);
    auto c = backgroundColour;
    
    if (isButtonDown)
        c = c.brighter(0.12f);
    else if (isMouseOverButton)
        c = c.brighter(0.06f);

    g.setColour(c);
    g.fillEllipse(r);

    // Ring stroke
    const float ringW = 2.5f;
    g.setColour(H9::accentTeal.withAlpha(
        (isButtonDown ? 0.45f : (isMouseOverButton ? 0.30f : 0.20f))));
    g.drawEllipse(r, ringW);

    // Subtle glow on hover
    if (isMouseOverButton || isButtonDown)
    {
        auto glow = r.expanded(6.0f);
        g.setColour(H9::accentTeal.withAlpha(isButtonDown ? 0.14f : 0.08f));
        g.fillEllipse(glow);
    }
}

void H9LookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b,
                                   bool, bool)
{
    g.setColour(juce::Colours::white);
    float f = (float)b.getHeight() * 0.30f;
    g.setFont(juce::Font(f, juce::Font::bold));

    auto r = b.getLocalBounds().toFloat();
    g.drawFittedText(b.getButtonText(), r.toNearestInt(), juce::Justification::centred, 1);
}
