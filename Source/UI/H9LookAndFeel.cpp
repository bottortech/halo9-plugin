#include "H9LookAndFeel.h"
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
//  H9LookAndFeel — Constructor
// ═══════════════════════════════════════════════════════════════════════════════

H9LookAndFeel::H9LookAndFeel()
{
    // ── Label ────────────────────────────────────────────────────────────
    setColour(juce::Label::textColourId,          H9::text);
    setColour(juce::Label::backgroundColourId,    juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId,       juce::Colours::transparentBlack);

    // ── TextEditor ───────────────────────────────────────────────────────
    setColour(juce::TextEditor::backgroundColourId,  H9::bg);
    setColour(juce::TextEditor::textColourId,        H9::text);
    setColour(juce::TextEditor::outlineColourId,     H9::border);
    setColour(juce::TextEditor::focusedOutlineColourId, H9::teal);
    setColour(juce::TextEditor::highlightColourId,   H9::teal.withAlpha(0.25f));
    setColour(juce::TextEditor::highlightedTextColourId, H9::text);
    setColour(juce::CaretComponent::caretColourId,   H9::teal);

    // ── PopupMenu ────────────────────────────────────────────────────────
    setColour(juce::PopupMenu::backgroundColourId,            H9::panel);
    setColour(juce::PopupMenu::textColourId,                  H9::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, H9::teal.withAlpha(0.2f));
    setColour(juce::PopupMenu::highlightedTextColourId,       H9::teal);
    setColour(juce::PopupMenu::headerTextColourId,            H9::dimText);

    // ── Tooltip ──────────────────────────────────────────────────────────
    setColour(juce::TooltipWindow::backgroundColourId, H9::panel);
    setColour(juce::TooltipWindow::textColourId,       H9::text);
    setColour(juce::TooltipWindow::outlineColourId,    H9::border);

    // ── ScrollBar ────────────────────────────────────────────────────────
    setColour(juce::ScrollBar::thumbColourId,          H9::border);
    setColour(juce::ScrollBar::trackColourId,          H9::bg);
    setColour(juce::ScrollBar::backgroundColourId,     H9::bg);

    // ── ListBox ──────────────────────────────────────────────────────────
    setColour(juce::ListBox::backgroundColourId,       H9::bg);
    setColour(juce::ListBox::outlineColourId,          H9::border);
    setColour(juce::ListBox::textColourId,             H9::text);

    // ── ComboBox ─────────────────────────────────────────────────────────
    setColour(juce::ComboBox::backgroundColourId,      H9::panel);
    setColour(juce::ComboBox::textColourId,            H9::text);
    setColour(juce::ComboBox::outlineColourId,         H9::border);
    setColour(juce::ComboBox::arrowColourId,           H9::dimText);
    setColour(juce::ComboBox::focusedOutlineColourId,  H9::teal);
    setColour(juce::ComboBox::buttonColourId,          H9::panel);

    // ── Slider ───────────────────────────────────────────────────────────
    setColour(juce::Slider::rotarySliderFillColourId,      H9::teal);
    setColour(juce::Slider::rotarySliderOutlineColourId,   H9::border);
    setColour(juce::Slider::thumbColourId,                 H9::teal);
    setColour(juce::Slider::textBoxTextColourId,           H9::dimText);
    setColour(juce::Slider::textBoxBackgroundColourId,     juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId,        juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxHighlightColourId,      H9::teal.withAlpha(0.25f));

    // ── TextButton ───────────────────────────────────────────────────────
    setColour(juce::TextButton::buttonColourId,    H9::panel);
    setColour(juce::TextButton::buttonOnColourId,  H9::teal.withAlpha(0.3f));
    setColour(juce::TextButton::textColourOffId,   H9::text);
    setColour(juce::TextButton::textColourOnId,    H9::teal);

    // ── AlertWindow ──────────────────────────────────────────────────────
    setColour(juce::AlertWindow::backgroundColourId, H9::panel);
    setColour(juce::AlertWindow::textColourId,       H9::text);
    setColour(juce::AlertWindow::outlineColourId,    H9::border);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  drawRotarySlider — arc-style knob with radial gradient glow
// ═══════════════════════════════════════════════════════════════════════════════

void H9LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                      int x, int y, int width, int height,
                                      float sliderPos,
                                      float rotaryStartAngle,
                                      float rotaryEndAngle,
                                      juce::Slider& /*slider*/)
{
    const float diameter = (float)juce::jmin(width, height) - 8.0f;
    const float radius   = diameter * 0.5f;
    const float cx       = (float)x + (float)width  * 0.5f;
    const float cy       = (float)y + (float)height * 0.5f;
    const float angle    = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float arcR     = radius - 4.0f;
    const float trackW   = 3.0f;
    const float valueW   = 3.5f;

    // ── 1) Multi-layer radial gradient glow ──────────────────────────────
    if (sliderPos > 0.01f)
    {
        const float alpha = 0.04f + sliderPos * 0.08f;
        const float glowR = radius + 6.0f;

        juce::ColourGradient glow(
            H9::teal.withAlpha(alpha), cx, cy,
            H9::teal.withAlpha(0.0f),  cx + glowR, cy, true);
        g.setGradientFill(glow);
        g.fillEllipse(cx - glowR, cy - glowR, glowR * 2.0f, glowR * 2.0f);
    }

    // ── 2) Knob body — gradient from center, lighter → panel ────────────
    {
        juce::ColourGradient body(
            H9::panel.brighter(0.04f), cx, cy,
            H9::panel,                 cx + radius, cy, true);
        g.setGradientFill(body);
        g.fillEllipse(cx - radius, cy - radius, diameter, diameter);
    }

    // ── Specular highlight (subtle) ───────────────────────────────────
    {
        const float specW = radius * 0.8f;
        const float specH = radius * 0.34f;
        juce::Rectangle<float> specR(cx - specW * 0.5f, cy - radius * 0.6f,
                                      specW, specH);
        juce::ColourGradient specGrad(juce::Colours::white.withAlpha(0.06f),
                                      specR.getX(), specR.getY(),
                                      juce::Colours::transparentWhite,
                                      specR.getRight(), specR.getBottom(), true);
        g.setGradientFill(specGrad);
        g.fillEllipse(specR);
    }

    // ── 3) Inner shadow ring ─────────────────────────────────────────────
    g.setColour(H9::bg.withAlpha(0.35f));
    g.drawEllipse(cx - radius + 1.0f, cy - radius + 1.0f,
                  diameter - 2.0f, diameter - 2.0f, 1.5f);

    // ── 4) Track arc (full sweep, muted) ─────────────────────────────────
    {
        juce::Path track;
        track.addCentredArc(cx, cy, arcR, arcR, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(H9::border);
        g.strokePath(track, juce::PathStrokeType(
            trackW, juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
    }

    // ── 5) Value arc (teal, thicker) ─────────────────────────────────────
    if (sliderPos > 0.005f)
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, arcR, arcR, 0.0f,
                          rotaryStartAngle, angle, true);
        g.setColour(H9::teal);
        g.strokePath(arc, juce::PathStrokeType(
            valueW, juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

        // End-cap dot at the arc terminus
        const float dotR = valueW * 0.7f;
        const float dotX = cx + arcR * std::cos(angle);
        const float dotY = cy + arcR * std::sin(angle);
        g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    }

    // ── 6) Pointer line from center toward current angle ─────────────────
    {
        const float lineLen = radius * 0.42f;
        g.setColour(H9::teal.withAlpha(0.65f));
        g.drawLine(cx, cy,
                   cx + lineLen * std::cos(angle),
                   cy + lineLen * std::sin(angle), 2.0f);
    }

    // ── 7) Center dot ────────────────────────────────────────────────────
    g.setColour(H9::teal);
    g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  drawButtonBackground — rounded rect with outer glow on press
// ═══════════════════════════════════════════════════════════════════════════════

void H9LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& bgColour,
                                           bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    const float corner = 5.0f;

    // Outer glow halo on press
    if (isButtonDown)
    {
        auto glowBounds = bounds.expanded(2.0f);
        g.setColour(H9::teal.withAlpha(0.12f));
        g.fillRoundedRectangle(glowBounds, corner + 2.0f);
    }

    // Fill
    auto fill = bgColour;
    if (isButtonDown)
        fill = H9::teal.withAlpha(0.2f);
    else if (isMouseOver)
        fill = bgColour.brighter(0.06f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, corner);

    // Border — teal on interaction, muted otherwise
    g.setColour((isMouseOver || isButtonDown) ? H9::teal.withAlpha(0.5f)
                                              : H9::border);
    g.drawRoundedRectangle(bounds, corner, 1.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  drawScrollbar — slim dark track, teal thumb
// ═══════════════════════════════════════════════════════════════════════════════

void H9LookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar&,
                                    int x, int y, int w, int h,
                                    bool isVertical, int thumbStart, int thumbSize,
                                    bool isMouseOver, bool isMouseDown)
{
    // Track
    g.setColour(H9::bg);
    g.fillRect(x, y, w, h);

    // Thumb
    juce::Rectangle<int> thumb;
    if (isVertical)
        thumb = { x + 2, thumbStart, w - 4, thumbSize };
    else
        thumb = { thumbStart, y + 2, thumbSize, h - 4 };

    auto colour = isMouseDown  ? H9::teal.withAlpha(0.5f)
                : isMouseOver  ? H9::teal.withAlpha(0.3f)
                               : H9::border;

    g.setColour(colour);
    g.fillRoundedRectangle(thumb.toFloat(), 3.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  drawTextEditorOutline — teal focus ring
// ═══════════════════════════════════════════════════════════════════════════════

void H9LookAndFeel::drawTextEditorOutline(juce::Graphics& g, int w, int h,
                                            juce::TextEditor& editor)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)w, (float)h);
    const float corner = 4.0f;

    if (editor.hasKeyboardFocus(true))
    {
        g.setColour(H9::teal.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.5f);
    }
    else
    {
        g.setColour(H9::border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.0f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  drawComboBox — dark body with teal border
// ═══════════════════════════════════════════════════════════════════════════════

void H9LookAndFeel::drawComboBox(juce::Graphics& g, int w, int h,
                                   bool isButtonDown,
                                   int /*buttonX*/, int /*buttonY*/,
                                   int /*buttonW*/, int /*buttonH*/,
                                   juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)w, (float)h);
    const float corner = 4.0f;

    // Body
    g.setColour(H9::panel);
    g.fillRoundedRectangle(bounds, corner);

    // Border — teal when focused or pressed
    const bool focused = box.hasKeyboardFocus(false);
    g.setColour((focused || isButtonDown) ? H9::teal.withAlpha(0.6f) : H9::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), corner, 1.0f);

    // Down-arrow chevron
    const float arrowSize = 6.0f;
    const float arrowX    = (float)w - 16.0f;
    const float arrowY    = (float)h * 0.5f;

    juce::Path arrow;
    arrow.addTriangle(arrowX - arrowSize, arrowY - arrowSize * 0.5f,
                      arrowX + arrowSize, arrowY - arrowSize * 0.5f,
                      arrowX,             arrowY + arrowSize * 0.5f);

    g.setColour(H9::dimText);
    g.fillPath(arrow);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════════

int H9LookAndFeel::getDefaultScrollbarWidth()
{
    return 10;
}

juce::Font H9LookAndFeel::getTextButtonFont(juce::TextButton&, int /*buttonHeight*/)
{
    return juce::Font(12.0f);
}
