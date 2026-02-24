#include "PluginEditor.h"
#include "EmbeddedHalo9.h"

// ═══════════════════════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════════════════════

HALO9PlayerAudioProcessorEditor::HALO9PlayerAudioProcessorEditor(
    HALO9PlayerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      masterAtt    (p.apvts, "master_volume",  masterSlider),
      cutoffAtt    (p.apvts, "lowpass_cutoff", cutoffSlider),
      atmosphereAtt(p.apvts, "atmosphere",     atmosphereSlider),
      synthLevelAtt(p.apvts, "synth_level",    synthLevelSlider),
      keyboardComponent(p.getKeyboardState(),
                        juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel);

    // ── Logo: try embedded memory first, then file system fallback ──────────
    if (halo9_png_len > 0)
        logoImage = juce::ImageCache::getFromMemory(halo9_png, halo9_png_len);

    if (!logoImage.isValid())
    {
        juce::File candidates[3];
        candidates[0] = juce::File::getCurrentWorkingDirectory()
                            .getChildFile("assets/branding/halo9.png");

        auto exe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        juce::File contents;
        if (exe.existsAsFile())
            contents = exe.getParentDirectory().getParentDirectory();
        auto resources = contents.getChildFile("Resources");

        candidates[1] = resources.getChildFile("halo9.png");
        candidates[2] = resources.getChildFile("assets/branding/halo9.png");

        for (int i = 0; i < 3; ++i)
        {
            if (candidates[i].existsAsFile())
            {
                auto img = juce::ImageCache::getFromFile(candidates[i]);
                if (img.isValid()) { logoImage = img; break; }
            }
        }
    }

    // ── Knob setup ─────────────────────────────────────────────────────────
    auto setupKnob = [&](juce::Slider& s, juce::Label& lbl, const char* name)
    {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        s.setPopupDisplayEnabled(true, true, this);
        addAndMakeVisible(s);

        lbl.setText(name, juce::dontSendNotification);
        lbl.setJustificationType(juce::Justification::centred);
        lbl.setColour(juce::Label::textColourId, H9::dimText);
        lbl.setFont(juce::Font(7.0f));
        addAndMakeVisible(lbl);
    };

    setupKnob(masterSlider,     masterLabel,     "MASTER");
    setupKnob(cutoffSlider,     cutoffLabel,     "LPF");
    setupKnob(atmosphereSlider, atmosphereLabel, "ATMOS");
    setupKnob(synthLevelSlider, synthLevelLabel, "SYNTH");

    // ── Circular pad buttons ───────────────────────────────────────────────
    for (int i = 0; i < NUM_PADS; ++i)
    {
        padButtons[i].setButtonText("P" + juce::String(i + 1));
        padButtons[i].setLookAndFeel(&circleLAF);
        padButtons[i].setColour(juce::TextButton::buttonColourId,
                                juce::Colour(0xff1a2228));
        padButtons[i].setColour(juce::TextButton::textColourOffId, H9::text);

        padButtons[i].onClick = [this, i]() { triggerPad(i); };

        padFlashEnd[i] = 0.0;
        addAndMakeVisible(padButtons[i]);
    }

    // ── Keyboard (HALO9 teal highlights) ──────────────────────────────────
    keyboardComponent.setColour(
        juce::MidiKeyboardComponent::whiteNoteColourId,
        juce::Colour(0xff27252d));
    keyboardComponent.setColour(
        juce::MidiKeyboardComponent::blackNoteColourId,
        juce::Colour(0xff0b0d12));
    keyboardComponent.setColour(
        juce::MidiKeyboardComponent::keySeparatorLineColourId,
        juce::Colour(0xff1e3634));
    keyboardComponent.setKeyWidth(23.0f);
    keyboardComponent.setAvailableRange(48, 84);
    updateKeyboardHighlight(activeKeyHighlightColor);
    addAndMakeVisible(keyboardComponent);

    // ── Library panel (populated from real data) ──────────────────────────
    auto& lib = processor.getLibrary();
    libraryPanel.populate(lib);

    libraryPanel.onPackSelected = [this](int index) { setActivePack(index); };
    libraryPanel.onKitSelected  = [this](int index) { setActiveKit(index); };

    addAndMakeVisible(libraryPanel);

    // ── Apply initial pack if available ───────────────────────────────────
    if (libraryPanel.selectedPack >= 0)
        setActivePack(libraryPanel.selectedPack);

    setWantsKeyboardFocus(true);
    startTimer(60);
    setOpaque(true);
    setSize(540, 760);

    // Defer window show/center until editor is attached to the host
    juce::MessageManager::callAsync([this]()
    {
        if (auto* top = dynamic_cast<juce::TopLevelWindow*>(getTopLevelComponent()))
        {
            top->centreWithSize(getWidth(), getHeight());
            top->setVisible(true);
            top->toFront(true);
        }
        else if (auto* comp = getTopLevelComponent())
        {
            comp->setVisible(true);
            comp->toFront(true);
        }
    });
}

HALO9PlayerAudioProcessorEditor::~HALO9PlayerAudioProcessorEditor()
{
    for (int i = 0; i < NUM_PADS; ++i)
        padButtons[i].setLookAndFeel(nullptr);

    setLookAndFeel(nullptr);
    stopTimer();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Active pack / kit selection
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::setActivePack(int index)
{
    auto& lib = processor.getLibrary();
    auto& packs = lib.getPacks();

    if (index < 0 || index >= (int)packs.size()) return;

    auto& pack = packs[(size_t)index];
    activePackId   = pack.id;
    activePackName = pack.name;
    activeAccentColor       = pack.accentColor;
    activeKeyHighlightColor = pack.keyHighlightColor;

    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (i < (int)pack.padBank.size())
            padButtons[i].setButtonText(pack.padBank[(size_t)i].label);
        else
            padButtons[i].setButtonText("P" + juce::String(i + 1));
    }

    circleLAF.glowColour = activeAccentColor;
    updateKeyboardHighlight(activeKeyHighlightColor);
    repaint();
}

void HALO9PlayerAudioProcessorEditor::setActiveKit(int index)
{
    auto& lib = processor.getLibrary();
    auto& kits = lib.getKits();

    if (index < 0 || index >= (int)kits.size())
    {
        activeKitId   = "";
        activeKitName = "No Kit";

        if (activePackId.isNotEmpty())
        {
            auto* pack = lib.findPack(activePackId);
            if (pack)
            {
                for (int i = 0; i < NUM_PADS; ++i)
                {
                    if (i < (int)pack->padBank.size())
                        padButtons[i].setButtonText(pack->padBank[(size_t)i].label);
                    else
                        padButtons[i].setButtonText("P" + juce::String(i + 1));
                }
            }
        }
        repaint();
        return;
    }

    auto& kit = kits[(size_t)index];
    activeKitId   = kit.id;
    activeKitName = kit.name;

    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (i < (int)kit.pads.size())
            padButtons[i].setButtonText(kit.pads[(size_t)i].label);
        else
            padButtons[i].setButtonText("P" + juce::String(i + 1));
    }

    repaint();
}

void HALO9PlayerAudioProcessorEditor::updateKeyboardHighlight(juce::Colour color)
{
    keyboardComponent.setColour(
        juce::MidiKeyboardComponent::keyDownOverlayColourId,
        color.withAlpha(0.35f));
    keyboardComponent.setColour(
        juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
        color.withAlpha(0.12f));
    keyboardComponent.repaint();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  hitTest — full window is opaque
// ═══════════════════════════════════════════════════════════════════════════════

bool HALO9PlayerAudioProcessorEditor::hitTest(int, int)
{
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Paint — Top Hub + premium background + disc + keyboard
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Solid neutral background
    g.fillAll(juce::Colour(0xff6f7476));

    discRadius = juce::jmax(50.0f, discRadius);

    // ── Ambient teal glow behind disc ────────────────────────────────────
    {
        float glowR = discRadius * 1.4f;
        juce::ColourGradient glow(
            activeAccentColor.withAlpha(0.05f),
            discCentre.x, discCentre.y,
            juce::Colours::transparentBlack,
            discCentre.x + glowR, discCentre.y,
            true);
        g.setGradientFill(glow);
        g.fillEllipse(discCentre.x - glowR, discCentre.y - glowR,
                      glowR * 2.0f, glowR * 2.0f);
    }

    // ── Soft vignette at edges ───────────────────────────────────────────
    {
        float vigR = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.55f;
        juce::ColourGradient vig(
            juce::Colours::transparentBlack,
            bounds.getCentreX(), bounds.getCentreY(),
            juce::Colours::black.withAlpha(0.22f),
            bounds.getCentreX() + vigR, bounds.getCentreY(),
            true);
        g.setGradientFill(vig);
        g.fillRect(bounds);
    }

    // ── Top Hub panel ────────────────────────────────────────────────────
    if (!hubBounds.isEmpty())
    {
        // Glass background
        juce::ColourGradient hubGrad(
            juce::Colour(0xff1c2129), hubBounds.getX(), hubBounds.getY(),
            juce::Colour(0xff12161c), hubBounds.getX(), hubBounds.getBottom(),
            false);
        g.setGradientFill(hubGrad);
        g.fillRoundedRectangle(hubBounds, 10.0f);

        // Teal stroke
        g.setColour(activeAccentColor.withAlpha(0.10f));
        g.drawRoundedRectangle(hubBounds, 10.0f, 0.8f);

        // Glass shine
        g.setColour(juce::Colours::white.withAlpha(0.02f));
        g.fillRoundedRectangle(
            hubBounds.reduced(1.5f).withHeight(hubBounds.getHeight() * 0.4f),
            9.0f);

        // Active pack name (left, top row)
        float textLeft = hubBounds.getX() + 14.0f;
        float textW = 130.0f;
        float textTop = hubBounds.getY() + 10.0f;

        // ── Logo area (below hub) ────────────────────────────────────────
        const float logoAreaHeight = 52.0f;
        juce::Rectangle<int> logoArea(0, (int)hubBounds.getBottom(),
                                       getWidth(), (int)logoAreaHeight);

        if (logoImage.isValid() && logoImage.getWidth() > 0 && logoImage.getHeight() > 0)
        {
            const int desiredH = juce::jmin(30, logoArea.getHeight());
            int imgW = logoImage.getWidth();
            int imgH = logoImage.getHeight();
            int targetW = (int)((float)imgW / (float)imgH * (float)desiredH);
            int dx = logoArea.getX() + (logoArea.getWidth() - targetW) / 2;
            int dy = logoArea.getY() + (logoArea.getHeight() - desiredH) / 2;
            g.drawImageWithin(logoImage, dx, dy, targetW, desiredH,
                              juce::RectanglePlacement::centred);
        }
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.65f));
            g.setFont(juce::Font(13.0f, juce::Font::bold));
            g.drawFittedText("HALO9", logoArea, juce::Justification::centred, 1);
        }

        g.setColour(activeAccentColor);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText(activePackName,
                   juce::Rectangle<float>(textLeft, textTop, textW, 14.0f),
                   juce::Justification::left, false);

        // Active kit name (left, bottom row)
        g.setColour(juce::Colour(0xff8b949e));
        g.setFont(juce::Font(9.0f));
        g.drawText(activeKitName,
                   juce::Rectangle<float>(textLeft, textTop + 16.0f, textW, 12.0f),
                   juce::Justification::left, false);

        // Admin indicator (right)
        if (libraryPanel.adminMode)
        {
            g.setColour(juce::Colour(0xffff6b6b).withAlpha(0.5f));
            g.fillEllipse(hubBounds.getRight() - 18.0f,
                          hubBounds.getCentreY() - 3.0f, 6.0f, 6.0f);

            g.setColour(juce::Colour(0xffff6b6b).withAlpha(0.35f));
            g.setFont(juce::Font(7.0f, juce::Font::bold));
            g.drawText("ADMIN",
                       juce::Rectangle<float>(hubBounds.getRight() - 50.0f,
                                              hubBounds.getCentreY() + 5.0f,
                                              40.0f, 10.0f),
                       juce::Justification::right, false);
        }
    }

    // ── Glassy keyboard panel ────────────────────────────────────────────
    auto kbBounds = keyboardComponent.getBounds().toFloat();
    if (!kbBounds.isEmpty())
    {
        auto glassArea = kbBounds.expanded(14.0f, 10.0f);

        juce::ColourGradient grad(
            juce::Colour(0xff1a1f26), glassArea.getX(), glassArea.getY(),
            juce::Colour(0xff0d1117), glassArea.getX(), glassArea.getBottom(),
            false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(glassArea, 12.0f);

        g.setColour(activeAccentColor.withAlpha(0.08f));
        g.drawRoundedRectangle(glassArea, 12.0f, 1.0f);

        g.setColour(juce::Colours::white.withAlpha(0.03f));
        auto shine = glassArea.reduced(1.5f).withHeight(glassArea.getHeight() * 0.3f);
        g.fillRoundedRectangle(shine, 11.0f);
    }

    // ── Disc fill (radial gradient for subtle depth) ─────────────────────
    {
        juce::ColourGradient discGrad(
            H9::bgLight.brighter(0.02f), discCentre.x, discCentre.y - discRadius * 0.18f,
            H9::panel.darker(0.02f),     discCentre.x, discCentre.y + discRadius, true);
        g.setGradientFill(discGrad);
        g.fillEllipse(discBounds);

        // Inner rim highlight
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        g.drawEllipse(discBounds.reduced(4.0f), 1.2f);

        // Outer teal ring (signature halo)
        g.setColour(activeAccentColor.withAlpha(0.14f));
        g.drawEllipse(discBounds.reduced(3.0f), 2.0f);
    }

    // ── Centered "H9" with refined typography ────────────────────────────
    {
        float baseFont = discBounds.getWidth() * 0.12f * 1.06f;

        // Shadow layer
        g.setColour(activeAccentColor.darker(0.25f).withAlpha(0.95f));
        g.setFont(juce::Font(baseFont + 2.0f, juce::Font::bold));
        auto shadowBounds = discBounds.translated(0.0f, 1.0f).toNearestInt();
        g.drawFittedText("H9", shadowBounds, juce::Justification::centred, 1);

        // Main layer
        g.setColour(activeAccentColor);
        g.setFont(juce::Font(baseFont, juce::Font::bold));
        g.drawFittedText("H9", discBounds.toNearestInt(), juce::Justification::centred, 1);

        // Subtle inner glow
        juce::Rectangle<int> textR = discBounds.toNearestInt();
        g.setColour(activeAccentColor.withAlpha(0.06f));
        g.fillEllipse((float)textR.getX() + textR.getWidth() * 0.25f,
                      (float)textR.getY(), (float)textR.getWidth() * 0.5f,
                      (float)textR.getHeight());
    }

    // ── Pad flash overlays (circles) ─────────────────────────────────────
    const double now = juce::Time::getMillisecondCounterHiRes();
    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (padFlashEnd[i] > now)
        {
            const float alpha = (float)(padFlashEnd[i] - now) / 120.0f;
            auto pb = padButtons[i].getBounds().toFloat().reduced(2.0f);
            g.setColour(activeAccentColor.withAlpha(juce::jmin(alpha * 0.35f, 0.35f)));
            g.fillEllipse(pb);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Layout — Top Hub + disc + pad ring + library + keyboard
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::resized()
{
    // ── Top Hub (90px height) ────────────────────────────────────────────
    const int hubH = 90;
    hubBounds = juce::Rectangle<float>(0.0f, 0.0f,
                                       (float)getWidth(), (float)hubH);

    // Knobs centered in hub
    {
        const int knobSize = 44;
        const int knobGap  = 10;
        const int numKnobs = 4;
        const int totalW   = knobSize * numKnobs + knobGap * (numKnobs - 1);
        const int startX   = (getWidth() - totalW) / 2;
        const int knobY    = (hubH - knobSize - 8) / 2;
        const int lblH     = 10;

        juce::Slider* sliders[] = { &masterSlider, &cutoffSlider,
                                    &atmosphereSlider, &synthLevelSlider };
        juce::Label*  labels[]  = { &masterLabel, &cutoffLabel,
                                    &atmosphereLabel, &synthLevelLabel };

        for (int i = 0; i < numKnobs; ++i)
        {
            int x = startX + i * (knobSize + knobGap);
            sliders[i]->setBounds(x, knobY, knobSize, knobSize);
            labels[i]->setBounds(x, knobY + knobSize - 1, knobSize, lblH);
        }
    }

    // Reserve keyboard at bottom
    const int kbMargin = 20;
    auto r = getLocalBounds();
    auto kbArea = r.removeFromBottom(160);
    keyboardComponent.setBounds(kbArea.reduced(kbMargin, 10));

    // Disc fills remaining space
    auto discArea = r.toFloat().reduced(16.0f);
    float diameter = juce::jmin(discArea.getWidth(), discArea.getHeight()) * 0.88f;
    discBounds = juce::Rectangle<float>(diameter, diameter)
                     .withCentre(discArea.getCentre());
    discCentre = discBounds.getCentre();
    discRadius = juce::jmax(50.0f, diameter * 0.5f);

    // ── 8 Pads in a perfect ring ─────────────────────────────────────────
    {
        float ringRadius = discRadius * 0.74f;
        int padSize = (int)(discBounds.getWidth() * 0.14f);

        for (int i = 0; i < NUM_PADS; ++i)
        {
            float angle = juce::MathConstants<float>::twoPi * (float)i / (float)NUM_PADS
                          - juce::MathConstants<float>::halfPi;

            juce::Point<float> pos {
                discCentre.x + std::cos(angle) * ringRadius,
                discCentre.y + std::sin(angle) * ringRadius
            };

            padButtons[i].setBounds(
                juce::Rectangle<float>((float)padSize, (float)padSize)
                    .withCentre(pos).toNearestInt());
        }
    }

    // ── Library panel centered between disc and keyboard ─────────────────
    {
        const int libW = getWidth() - 40;
        const int libH = 90;
        int libX = (getWidth() - libW) / 2;
        int discBot = (int)discBounds.getBottom();
        int kbTop = kbArea.getY();
        int libY = discBot + (kbTop - discBot - libH) / 2;
        libraryPanel.setBounds(libX, libY, libW, libH);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Keyboard — keys 1-8 trigger pads, Cmd+Shift+L toggles admin
// ═══════════════════════════════════════════════════════════════════════════════

bool HALO9PlayerAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if ((key.getKeyCode() == 'l' || key.getKeyCode() == 'L')
        && key.getModifiers().isCommandDown()
        && key.getModifiers().isShiftDown())
    {
        libraryPanel.adminMode = !libraryPanel.adminMode;
        libraryPanel.repaint();
        repaint();
        return true;
    }

    const int code = key.getTextCharacter();
    if (code >= '1' && code <= '8')
    {
        triggerPad(code - '1');
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Pad helpers
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::triggerPad(int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS) return;

    // Visual flash (audio trigger will be added when PadSampler is restored)
    padFlashEnd[padIndex] = juce::Time::getMillisecondCounterHiRes() + 120.0;
    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Timer — repaint during pad flash animations
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (padFlashEnd[i] > now)
        {
            repaint();
            return;
        }
    }
}
