#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "UI/H9LookAndFeel.h"
#include "Data/H9Library.h"

// ── HALO9 Instrument Editor ─────────────────────────────────────────────────

class HALO9PlayerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit HALO9PlayerAudioProcessorEditor(HALO9PlayerAudioProcessor&);
    ~HALO9PlayerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    bool hitTest(int x, int y) override;

private:
    HALO9PlayerAudioProcessor& processor;
    H9LookAndFeel lookAndFeel;

    // ── Circular button LookAndFeel ──────────────────────────────────────────
    struct CircleButtonLAF : juce::LookAndFeel_V4
    {
        juce::Colour glowColour { 0xff2ee6c9 };

        void drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                  const juce::Colour& backgroundColour,
                                  bool isMouseOverButton, bool isButtonDown) override
        {
            auto r = b.getLocalBounds().toFloat().reduced(2.0f);
            auto c = backgroundColour;
            if (isButtonDown)            c = c.brighter(0.12f);
            else if (isMouseOverButton)  c = c.brighter(0.06f);

            g.setColour(c);
            g.fillEllipse(r);

            const float ringW = 2.5f;
            g.setColour(glowColour.withAlpha((isButtonDown ? 0.45f : (isMouseOverButton ? 0.30f : 0.20f))));
            g.drawEllipse(r, ringW);

            if (isMouseOverButton || isButtonDown)
            {
                auto glow = r.expanded(6.0f);
                g.setColour(glowColour.withAlpha(isButtonDown ? 0.14f : 0.08f));
                g.fillEllipse(glow);
            }
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& b,
                            bool, bool) override
        {
            g.setColour(juce::Colours::white);
            float f = (float)b.getHeight() * 0.30f;
            g.setFont(juce::Font(f, juce::Font::bold));
            auto r = b.getLocalBounds().toFloat();
            g.drawFittedText(b.getButtonText(), r.toNearestInt(), juce::Justification::centred, 1);
        }
    };

    CircleButtonLAF circleLAF;

    // ── Logo image ──────────────────────────────────────────────────────────
    juce::Image logoImage;

    // ── Circular pad button (circular hitTest) ──────────────────────────────
    struct CirclePadButton : juce::TextButton
    {
        bool hitTest(int x, int y) override
        {
            auto r = getLocalBounds().toFloat().reduced(2.0f);
            auto dx = (float)x - r.getCentreX();
            auto dy = (float)y - r.getCentreY();
            auto rad = juce::jmin(r.getWidth(), r.getHeight()) * 0.5f;
            return (dx * dx + dy * dy) <= (rad * rad);
        }
    };

    // ── Library panel (driven by H9Library data) ─────────────────────────────
    struct LibraryPanel : juce::Component
    {
        juce::StringArray packNames;
        juce::StringArray kitNames;
        juce::StringArray packIds;
        juce::StringArray kitIds;

        int  selectedPack  { -1 };
        int  selectedKit   { -1 };
        bool hovering      { false };
        int  hoverChipId   { -1 };
        bool adminMode     { false };

        std::vector<juce::Rectangle<float>> packRects;
        std::vector<juce::Rectangle<float>> kitRects;

        std::function<void(int)> onPackSelected;
        std::function<void(int)> onKitSelected;

        void populate(const H9Library& lib)
        {
            packNames.clear(); packIds.clear();
            kitNames.clear();  kitIds.clear();

            for (auto& p : lib.getPacks())
            {
                packNames.add(p.name);
                packIds.add(p.id);
            }
            for (auto& k : lib.getKits())
            {
                kitNames.add(k.name);
                kitIds.add(k.id);
            }

            selectedPack = packNames.isEmpty() ? -1 : 0;
            selectedKit = -1;
            repaint();
        }

        void mouseEnter(const juce::MouseEvent&) override { hovering = true;  repaint(); }
        void mouseExit (const juce::MouseEvent&) override { hovering = false; hoverChipId = -1; repaint(); }

        void mouseMove(const juce::MouseEvent& e) override
        {
            int prev = hoverChipId;
            hoverChipId = -1;
            for (int i = 0; i < (int)packRects.size(); ++i)
                if (packRects[(size_t)i].contains(e.position)) { hoverChipId = i; break; }
            if (hoverChipId < 0)
                for (int i = 0; i < (int)kitRects.size(); ++i)
                    if (kitRects[(size_t)i].contains(e.position)) { hoverChipId = 100 + i; break; }
            if (hoverChipId != prev) repaint();
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            for (int i = 0; i < (int)packRects.size(); ++i)
            {
                if (packRects[(size_t)i].contains(e.position))
                {
                    selectedPack = i;
                    if (onPackSelected) onPackSelected(i);
                    repaint();
                    return;
                }
            }
            for (int i = 0; i < (int)kitRects.size(); ++i)
            {
                if (kitRects[(size_t)i].contains(e.position))
                {
                    selectedKit = (selectedKit == i) ? -1 : i;
                    if (onKitSelected) onKitSelected(selectedKit);
                    repaint();
                    return;
                }
            }
        }

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(1.0f);

            g.setColour(juce::Colours::black.withAlpha(0.15f));
            g.fillRoundedRectangle(b.translated(0.0f, 2.0f), 10.0f);

            juce::ColourGradient grad(
                juce::Colour(0xff1c2129), b.getX(), b.getY(),
                juce::Colour(0xff12161c), b.getX(), b.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(b, 10.0f);

            float glowA = hovering ? 0.20f : 0.08f;
            g.setColour(juce::Colour(0xff2ee6c9).withAlpha(glowA));
            g.drawRoundedRectangle(b.reduced(0.5f), 9.5f, 0.8f);

            g.setColour(juce::Colour(0xff2ee6c9).withAlpha(hovering ? 0.16f : 0.10f));
            g.drawRoundedRectangle(b, 10.0f, 1.0f);

            const float pad = 12.0f;
            float y = b.getY() + 8.0f;
            float left = b.getX() + pad;
            float w = b.getWidth() - pad * 2.0f;
            const float chipH = 19.0f;
            const float chipGap = 4.0f;

            auto drawChips = [&](const juce::StringArray& names,
                                 std::vector<juce::Rectangle<float>>& rects,
                                 int selected, int idOffset,
                                 const char* label, const char* emptyMsg)
            {
                g.setColour(juce::Colour(0xff6e7681));
                g.setFont(juce::Font(8.0f, juce::Font::bold));
                g.drawText(juce::String(label),
                           juce::Rectangle<float>(left, y, w, 10.0f),
                           juce::Justification::left, false);
                y += 13.0f;

                rects.clear();
                if (names.isEmpty())
                {
                    g.setColour(juce::Colour(0xff6e7681).withAlpha(0.5f));
                    g.setFont(juce::Font(9.0f));
                    g.drawText(juce::String(emptyMsg),
                               juce::Rectangle<float>(left, y, w, chipH),
                               juce::Justification::left, false);
                    y += chipH + 8.0f;
                    return;
                }

                float chipX = left;
                g.setFont(juce::Font(9.0f));

                for (int i = 0; i < names.size(); ++i)
                {
                    float chipW = g.getCurrentFont().getStringWidthFloat(names[i]) + 14.0f;
                    auto cr = juce::Rectangle<float>(chipX, y, chipW, chipH);
                    rects.push_back(cr);

                    bool sel = (i == selected);
                    bool hov = (hoverChipId == (idOffset + i));

                    if (sel)
                    {
                        g.setColour(juce::Colour(0xff2ee6c9).withAlpha(0.18f));
                        g.fillRoundedRectangle(cr, 5.0f);
                        g.setColour(juce::Colour(0xff2ee6c9).withAlpha(0.35f));
                        g.drawRoundedRectangle(cr, 5.0f, 0.8f);
                    }
                    else
                    {
                        g.setColour(juce::Colour(0xff1a2228).withAlpha(hov ? 0.9f : 0.5f));
                        g.fillRoundedRectangle(cr, 5.0f);
                        if (hov)
                        {
                            g.setColour(juce::Colour(0xff30363d));
                            g.drawRoundedRectangle(cr, 5.0f, 0.5f);
                        }
                    }

                    g.setColour(sel ? juce::Colour(0xff2ee6c9) :
                                juce::Colour(0xffe6edf3).withAlpha(hov ? 0.9f : 0.6f));
                    g.drawText(names[i], cr, juce::Justification::centred);

                    chipX += chipW + chipGap;
                }

                if (adminMode)
                {
                    float plusW = 24.0f;
                    auto plusR = juce::Rectangle<float>(chipX + 2.0f, y, plusW, chipH);
                    g.setColour(juce::Colour(0xff1a2228).withAlpha(0.35f));
                    g.fillRoundedRectangle(plusR, 5.0f);
                    g.setColour(juce::Colour(0xff2ee6c9).withAlpha(0.25f));
                    g.setFont(juce::Font(12.0f));
                    g.drawText("+", plusR, juce::Justification::centred);
                }

                y += chipH + 8.0f;
            };

            drawChips(packNames, packRects, selectedPack, 0,
                      "SOUND PACKS", "No packs installed");
            drawChips(kitNames,  kitRects,  selectedKit,  100,
                      "DRUM KITS", "No drum kits installed");

            if (adminMode)
            {
                g.setColour(juce::Colour(0xffff6b6b).withAlpha(0.50f));
                g.fillEllipse(b.getRight() - 14.0f, b.getY() + 6.0f, 5.0f, 5.0f);
            }
        }
    };

    LibraryPanel libraryPanel;

    // ── Active pack/kit state ────────────────────────────────────────────────
    juce::String activePackId;
    juce::String activeKitId;
    juce::String activePackName   { "No Pack" };
    juce::String activeKitName    { "No Kit" };
    juce::Colour activeAccentColor       { 0xff33ffc8 };
    juce::Colour activeKeyHighlightColor { 0xff33ffc8 };

    void setActivePack(int index);
    void setActiveKit(int index);

    // ── Disc geometry (computed in resized) ──────────────────────────────────
    juce::Rectangle<float> discBounds;
    juce::Point<float>     discCentre;
    float                  discRadius { 0.0f };

    // ── Top Hub layout rect ─────────────────────────────────────────────────
    juce::Rectangle<float> hubBounds;

    // ── Parameter knobs ─────────────────────────────────────────────────────
    juce::Slider masterSlider, cutoffSlider, atmosphereSlider, synthLevelSlider;
    juce::Label  masterLabel,  cutoffLabel,  atmosphereLabel,  synthLevelLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment
        masterAtt, cutoffAtt, atmosphereAtt, synthLevelAtt;

    // ── Circular pad buttons ────────────────────────────────────────────────
    static constexpr int NUM_PADS = 8;
    CirclePadButton padButtons[NUM_PADS];
    double padFlashEnd[NUM_PADS] {};

    // ── Keyboard ────────────────────────────────────────────────────────────
    juce::MidiKeyboardComponent keyboardComponent;

    // ── Helpers ─────────────────────────────────────────────────────────────
    void triggerPad(int padIndex);
    void updateKeyboardHighlight(juce::Colour color);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HALO9PlayerAudioProcessorEditor)
};
