#include "PluginEditor.h"
#include "EmbeddedHalo9.h"
#include <iostream>

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
    // First try embedded memory (BinaryData / generated array)
    if (halo9_png_len > 0)
    {
        logoImage = juce::ImageCache::getFromMemory(halo9_png, halo9_png_len);
        DBG("embedded logo loaded: " + juce::String(logoImage.isValid() ? "true" : "false") + " size: " + juce::String(logoImage.getWidth()) + "x" + juce::String(logoImage.getHeight()));
    }

    // If embedded not available, fall back to file-system / bundle Resources lookup
    if (!logoImage.isValid())
    {
        // 1) Workspace path (use current working dir)
        juce::File candidates[3];
        candidates[0] = juce::File::getCurrentWorkingDirectory().getChildFile("assets/branding/halo9.png");

        // Prepare bundle Resources base (Contents/Resources)
        auto exe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        juce::File contents;
        if (exe.existsAsFile()) contents = exe.getParentDirectory().getParentDirectory(); // Contents
        auto resources = contents.getChildFile("Resources");

        // 2) Bundled Resources: Contents/Resources/halo9.png
        candidates[1] = resources.getChildFile("halo9.png");

        // 3) Bundled Resources: Contents/Resources/assets/branding/halo9.png
        candidates[2] = resources.getChildFile("assets").getChildFile("branding").getChildFile("halo9.png");

        for (int i = 0; i < 3; ++i)
        {
            auto f = candidates[i];
            juce::String p = f.getFullPathName();
            bool exists = f.existsAsFile();
            DBG("logo candidate: " + p);
            DBG("  exists: " + juce::String(exists ? "true" : "false"));

            bool loaded = false;
            juce::Image img;
            if (exists)
            {
                img = juce::ImageCache::getFromFile(f);
                loaded = img.isValid();
            }

            juce::String sizeStr = loaded ? (juce::String(img.getWidth()) + "x" + juce::String(img.getHeight())) : "0x0";
            DBG("  loaded: " + juce::String(loaded ? "true" : "false") + " size: " + sizeStr);

            if (loaded)
            {
                logoImage = img;
                break;
            }
        }

        if (!logoImage.isValid())
            DBG("HALO9 logo not found in workspace or bundle Resources (checked 3 candidate paths).");
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
        padButtons[i].setColour(juce::TextButton::textColourOffId,
                                H9::text);

        padButtons[i].onClick = [this, i]()
        {
            if (processor.padSamplePaths[i].isNotEmpty())
            {
                triggerPad(i);
            }
            else
            {
                auto chooser = std::make_shared<juce::FileChooser>(
                    "Load sample for Pad " + juce::String(i + 1),
                    juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                    "*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg");

                chooser->launchAsync(
                    juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles,
                    [this, i, chooser](const juce::FileChooser& fc)
                    {
                        auto result = fc.getResult();
                        if (result.existsAsFile())
                        {
                            processor.getPadSampler().loadSample(i, result);
                            processor.padSamplePaths[i] = result.getFullPathName();
                            padButtons[i].setButtonText(
                                result.getFileNameWithoutExtension()
                                    .substring(0, 5));
                            setStatus("Pad " + juce::String(i + 1)
                                      + ": " + result.getFileName());
                        }
                    });
            }
        };

        padFlashEnd[i] = 0.0;
        addAndMakeVisible(padButtons[i]);
    }

    updatePadButtonLabels();

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

    // Ensure a sane initial size and log startup
    std::cout << "HALO9 Player: editor constructed\n";
    std::cout.flush();
    setOpaque(true);
    setSize(540, 760); // enough space for circle + keyboard

    // Defer top-level window adjustments until the editor is attached
    juce::MessageManager::callAsync([this]()
    {
        std::cout << "HALO9 Player: attempting to show and center window\n";
        std::cout.flush();
        if (auto* top = dynamic_cast<juce::TopLevelWindow*>(getTopLevelComponent()))
        {
            top->centreWithSize(getWidth(), getHeight());
            top->setVisible(true);
            top->toFront(true);
            std::cout << "HALO9 Player: window centered and shown\n";
            std::cout.flush();
        }
        else if (auto* comp = getTopLevelComponent())
        {
            comp->setVisible(true);
            comp->toFront(true);
            std::cout << "HALO9 Player: top-level component shown\n";
            std::cout.flush();
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

    // Update pad labels from pack padBank
    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (i < (int)pack.padBank.size())
            padButtons[i].setButtonText(pack.padBank[(size_t)i].label);
        else
            padButtons[i].setButtonText("P" + juce::String(i + 1));
    }

    // Update pad glow color
    circleLAF.glowColour = activeAccentColor;

    // Update keyboard highlight to match pack accent
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

        // Restore pad labels from active pack (if any)
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

    // Update pad labels from kit mapping
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
//  Arc placement helper
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::placeOnArc(
    juce::Component& comp, float angleRad, int w, int h, float radius)
{
    const float cx = discCentre.x + std::cos(angleRad) * radius - w * 0.5f;
    const float cy = discCentre.y + std::sin(angleRad) * radius - h * 0.5f;
    comp.setBounds((int)cx, (int)cy, w, h);
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

    // ── Premium gradient background (match disc/top-hub tones for cohesion) ──
    {
        // Use the panel/disc tones so the outer sheet reads as one surface
        juce::Colour c1 = H9::panel.darker(0.06f);
        juce::Colour c2 = H9::panel.darker(0.16f);
        juce::ColourGradient bgGrad(c1, discCentre.x, discCentre.y,
                                   c2, bounds.getCentreX() + bounds.getWidth() * 0.35f, bounds.getCentreY(), true);
        g.setGradientFill(bgGrad);
        g.fillRect(bounds);
    }

    // Ensure the disc always has a usable radius (avoid early return)
    discRadius = juce::jmax(50.0f, discRadius);

    // ── Layered teal glow behind disc (soft halo, softened for premium feel) ─
    {
        // Outermost glow: large soft bloom
        float glowR1 = discRadius * 1.30f;
        juce::ColourGradient glow1(
            activeAccentColor.withAlpha(0.06f),
            discCentre.x, discCentre.y,
            juce::Colours::transparentBlack,
            discCentre.x + glowR1, discCentre.y,
            true);
        g.setGradientFill(glow1);
        g.fillEllipse(discCentre.x - glowR1, discCentre.y - glowR1,
                      glowR1 * 2.0f, glowR1 * 2.0f);

        // Mid glow: medium size, slightly brighter but still soft
        float glowR2 = discRadius * 1.12f;
        juce::ColourGradient glow2(
            activeAccentColor.withAlpha(0.10f),
            discCentre.x, discCentre.y,
            juce::Colours::transparentBlack,
            discCentre.x + glowR2, discCentre.y,
            true);
        g.setGradientFill(glow2);
        g.fillEllipse(discCentre.x - glowR2, discCentre.y - glowR2,
                      glowR2 * 2.0f, glowR2 * 2.0f);

        // Inner glow: smallest, very subtle
        float glowR3 = discRadius * 1.03f;
        g.setColour(activeAccentColor.withAlpha(0.05f));
        g.fillEllipse(discCentre.x - glowR3, discCentre.y - glowR3,
                      glowR3 * 2.0f, glowR3 * 2.0f);
    }

    // ── Soft vignette at edges (darken corners) ──────────────────────────
    {
        float vigR = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.55f;
        juce::ColourGradient vig(
            juce::Colours::transparentBlack,
            bounds.getCentreX(), bounds.getCentreY(),
            juce::Colours::black.withAlpha(0.18f),
            bounds.getCentreX() + vigR, bounds.getCentreY(),
            true);
        g.setGradientFill(vig);
        g.fillRect(bounds);
    }

    // ── Top Hub panel ────────────────────────────────────────────────────
    if (!hubBounds.isEmpty())
    {
        // Glass background for hub
        juce::ColourGradient hubGrad(
            H9::panel.brighter(0.02f), hubBounds.getX(), hubBounds.getY(),
            H9::panel.darker(0.06f), hubBounds.getX(), hubBounds.getBottom(),
            false);
        g.setGradientFill(hubGrad);
        g.fillRoundedRectangle(hubBounds, 10.0f);

        // Teal stroke for hub container (very subtle)
        g.setColour(activeAccentColor.withAlpha(0.08f));
        g.drawRoundedRectangle(hubBounds, 10.0f, 0.6f);

        // Glass shine
        g.setColour(juce::Colours::white.withAlpha(0.02f));
        g.fillRoundedRectangle(
            hubBounds.reduced(1.5f).withHeight(hubBounds.getHeight() * 0.4f),
            9.0f);

        // --- Knob module background (panel behind knobs) -----------------
        // Compute union of knob bounds so the panel sizes responsively
        juce::Rectangle<float> knobsUnion;
        {
            juce::Slider* slidersArr[] = { &masterSlider, &cutoffSlider, &atmosphereSlider, &synthLevelSlider };
            knobsUnion = slidersArr[0]->getBounds().toFloat();
            for (int i = 1; i < 4; ++i)
                knobsUnion = knobsUnion.getUnion(slidersArr[i]->getBounds().toFloat());
        }

        // Padding relative to knob height
        float padX = juce::jmax(12.0f, knobsUnion.getHeight() * 0.6f);
        float padY = juce::jmax(6.0f, knobsUnion.getHeight() * 0.35f);
        auto moduleRect = knobsUnion.expanded(padX, padY);
        float corner = juce::jmin(14.0f, moduleRect.getHeight() * 0.45f);

        // Soft shadow below module
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.fillRoundedRectangle(moduleRect.translated(0.0f, 3.0f), corner);

        // Module background (glassy)
        juce::ColourGradient moduleGrad(
            H9::panel.brighter(0.03f), moduleRect.getX(), moduleRect.getY(),
            H9::panel.darker(0.08f), moduleRect.getX(), moduleRect.getBottom(),
            false);
        g.setGradientFill(moduleGrad);
        g.fillRoundedRectangle(moduleRect, corner);

        // Thin teal outline for module
        g.setColour(activeAccentColor.withAlpha(0.10f));
        g.drawRoundedRectangle(moduleRect, corner, 0.9f);

        // Pack name display removed (consolidated into Now Playing panel)

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

        // ── Now Playing digital panel (right side of hub) ───────────────
        {
            // Panel width responsive to hub width, but never too large
            float panelW = juce::jmin(160.0f, hubBounds.getWidth() * 0.26f);
            float panelH = juce::jmax(44.0f, hubBounds.getHeight() * 0.58f);
            // Provide more spacing from the knob module so they read as distinct modules
            float spacingRight = 28.0f; // base spacing
            if (!moduleRect.isEmpty())
                spacingRight = juce::jmax(spacingRight, moduleRect.getWidth() * 0.12f);
            float panelX = hubBounds.getRight() - panelW - spacingRight;
            float panelY = hubBounds.getY() + (hubBounds.getHeight() - panelH) * 0.5f;
            auto panelBounds = juce::Rectangle<float>(panelX, panelY, panelW, panelH);

            // If the panel would overlap the knob module, nudge it right of the module instead
            if (!moduleRect.isEmpty() && panelBounds.intersects(moduleRect))
            {
                panelBounds.setX(moduleRect.getRight() + spacingRight);
            }

            // Subtle drop shadow so Now Playing reads as its own module
            g.setColour(juce::Colours::black.withAlpha(0.10f));
            g.fillRoundedRectangle(panelBounds.translated(0.0f, 2.0f), 6.0f);

            // Dark glass background
            juce::ColourGradient panelGrad(
                juce::Colour(0xff1f2428), panelBounds.getX(), panelBounds.getY(),
                juce::Colour(0xff151a1e), panelBounds.getX(), panelBounds.getBottom(),
                false);
            g.setGradientFill(panelGrad);
            g.fillRoundedRectangle(panelBounds, 6.0f);

            // Teal accent border
            g.setColour(activeAccentColor.withAlpha(0.15f));
            g.drawRoundedRectangle(panelBounds, 6.0f, 0.8f);

            // Inner shine
            g.setColour(juce::Colours::white.withAlpha(0.03f));
            g.fillRoundedRectangle(
                panelBounds.reduced(1.0f).withHeight(panelBounds.getHeight() * 0.35f),
                5.0f);

            // Label: "NOW PLAYING"
            g.setColour(activeAccentColor.withAlpha(0.7f));
            g.setFont(juce::Font(7.0f, juce::Font::bold));
            g.drawText("NOW PLAYING",
                       panelBounds.withHeight(12.0f).reduced(4.0f, 0.0f),
                       juce::Justification::left, false);

            // Current sound name (pack name)
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.drawText(activePackName,
                       panelBounds.withTrimmedTop(12.0f).withHeight(18.0f).reduced(4.0f, 2.0f),
                       juce::Justification::left, false);

            // Kit name (smaller text)
            g.setColour(juce::Colour(0xff8b949e));
            g.setFont(juce::Font(8.0f));
            g.drawText(activeKitName.isEmpty() ? "No Kit" : activeKitName,
                       panelBounds.withTrimmedTop(30.0f).reduced(4.0f, 2.0f),
                       juce::Justification::left, false);
        }
    }

    // ── Disc fill (radial gradient for subtle depth + 3D feel) ───────────
    {
        juce::ColourGradient discGrad(
            H9::bgLight.brighter(0.02f), discCentre.x, discCentre.y - discRadius * 0.18f,
            H9::panel.darker(0.02f),        discCentre.x, discCentre.y + discRadius, true);
        g.setGradientFill(discGrad);
        g.fillEllipse(discBounds);

        // Top-left subtle highlight (dimensional effect)
        const float highlightAngle = -45.0f * juce::MathConstants<float>::pi / 180.0f;
        const float highlightDist = discRadius * 0.65f;
        const float hlX = discCentre.x + std::cos(highlightAngle) * highlightDist;
        const float hlY = discCentre.y + std::sin(highlightAngle) * highlightDist;
        juce::ColourGradient hlGrad(
            juce::Colours::white.withAlpha(0.08f), hlX, hlY,
            juce::Colours::transparentBlack, discCentre.x, discCentre.y, false);
        g.setGradientFill(hlGrad);
        g.fillEllipse(discBounds);

        // Bottom-right subtle shadow (dimensional effect)
        const float shadowAngle = 135.0f * juce::MathConstants<float>::pi / 180.0f;
        const float shadowDist = discRadius * 0.7f;
        const float shX = discCentre.x + std::cos(shadowAngle) * shadowDist;
        const float shY = discCentre.y + std::sin(shadowAngle) * shadowDist;
        juce::ColourGradient shGrad(
            juce::Colours::black.withAlpha(0.06f), shX, shY,
            juce::Colours::transparentBlack, discCentre.x, discCentre.y, false);
        g.setGradientFill(shGrad);
        g.fillEllipse(discBounds);

        // Inner rim highlight
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        g.drawEllipse(discBounds.reduced(4.0f), 1.2f);

        // Soft outer teal ring (signature halo)
        g.setColour(activeAccentColor.withAlpha(0.12f));
        g.drawEllipse(discBounds.reduced(3.0f), 2.0f);
    }

    // ── Horizontal bar removed as requested ──

    // ── Glassy keyboard panel (premium glass aesthetic) ──────────────────
    auto kbBounds = keyboardComponent.getBounds().toFloat();
    if (!kbBounds.isEmpty())
    {
        auto glassArea = kbBounds.expanded(14.0f, 10.0f);

        // Soft shadow above keyboard (detachment effect)
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.fillRoundedRectangle(glassArea.translated(0.0f, -4.0f).withHeight(4.0f), 12.0f);

        // Glass background gradient (charcoal tones)
        juce::ColourGradient grad(
            juce::Colour(0xff2a2f35), glassArea.getX(), glassArea.getY(),
            juce::Colour(0xff1a1f26), glassArea.getX(), glassArea.getBottom(),
            false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(glassArea, 12.0f);

        // Teal accent stroke
        g.setColour(activeAccentColor.withAlpha(0.10f));
        g.drawRoundedRectangle(glassArea, 12.0f, 1.0f);

        // Subtle top shine (glass effect)
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        auto shine = glassArea.reduced(1.5f).withHeight(glassArea.getHeight() * 0.35f);
        g.fillRoundedRectangle(shine, 11.0f);
    }



    // ── Pad flash overlays (circles) ─────────────────────────────────────
    // ── Centered "H9" text inside the disc (teal) ─────────────────────
    {
        float baseFont = discBounds.getWidth() * 0.12f * 1.06f; // +6% perceived weight

        // Back layer (slightly darker, offset for perceived weight)
        g.setColour(activeAccentColor.darker(0.25f).withAlpha(0.95f));
        g.setFont(juce::Font(baseFont + 2.0f, juce::Font::bold));
        auto shadowBounds = discBounds.translated(0.0f, 1.0f).toNearestInt();
        g.drawFittedText("H9", shadowBounds, juce::Justification::centred, 1);

        // Front layer (main)
        g.setColour(activeAccentColor);
        g.setFont(juce::Font(baseFont, juce::Font::bold));
        g.drawFittedText("H9", discBounds.toNearestInt(), juce::Justification::centred, 1);

        // Very subtle inner glow: low-alpha small radial tint over text
        juce::Rectangle<int> textR = discBounds.toNearestInt();
        g.setColour(activeAccentColor.withAlpha(0.06f));
        g.fillEllipse((float)textR.getX() + textR.getWidth() * 0.25f,
                      (float)textR.getY(), (float)textR.getWidth() * 0.5f,
                      (float)textR.getHeight());
    }

    // ── halo9.png overlay: draw last so it sits outside the disc (upper-left badge)
    if (logoImage.isValid() && logoImage.getWidth() > 0 && logoImage.getHeight() > 0)
    {
        // Determine logo width relative to disc radius (1.8x-2.5x bigger)
        const float margin = 18.0f;
        int imgW = logoImage.getWidth();
        int imgH = logoImage.getHeight();
        int logoW = (int)std::round(discRadius * 0.65f);  // Increased from 0.35f to 0.65f (1.86x larger)
        int logoH = juce::jmax(2, (int)std::round((float)imgH / (float)imgW * (float)logoW));

        // Per requested placement (upper-left outside the disc as a badge)
        float logoX = discBounds.getX() - ((float)logoW * 0.40f);
        float logoY = discBounds.getY() + (discBounds.getHeight() * 0.08f);

        // Clamp so it doesn't clip editor bounds
        logoX = juce::jmax(bounds.getX() + 12.0f, logoX);
        logoY = juce::jlimit(bounds.getY() + 12.0f, bounds.getBottom() - (float)logoH - 12.0f, logoY);

        // Draw at full opacity (100%, not faint)
        g.saveState();
        g.setOpacity(1.0f);
        g.drawImageWithin(logoImage, (int)logoX, (int)logoY, logoW, logoH, juce::RectanglePlacement::stretchToFit);
        g.restoreState();
    }
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
    auto fullBounds = getLocalBounds().toFloat();

    // ── Top Hub (explicit 90px height as requested) ──────────────────────
    const int hubH = 90;
    const int hubTopMargin = 0;
    hubBounds = juce::Rectangle<float>(0.0f, 0.0f,
                                       (float)getWidth(),
                                       (float)hubH);

    // Knobs in hub center
    {
        const int knobSize = 44;
        const int knobGap  = 10;
        const int numKnobs = 4;
        const int totalW   = knobSize * numKnobs + knobGap * (numKnobs - 1);
        const int startX   = (getWidth() - totalW) / 2;
        const int knobY    = hubTopMargin + (hubH - knobSize - 8) / 2;
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

    // Reserve a transparent logo area under the hub (y=90, h=52)
    const int logoAreaH = 52;

    // Reserve a fixed keyboard area at the bottom so layout cannot collapse
    const int kbMargin = 20;
    auto r = getLocalBounds();
    auto kbArea = r.removeFromBottom(160);  // keyboard height
    keyboardComponent.setBounds(kbArea.reduced(kbMargin, 10));

    // Use the remaining area for the disc sizing math
    auto discArea = r.toFloat().reduced(16.0f);

    // Slightly reduce the disc size so it breathes under the hub (approx 10-15%)
    const float reduction = 0.88f * 0.88f; // tuned reduction factor
    float diameter = juce::jmin(discArea.getWidth(), discArea.getHeight()) * reduction;

    // Shift disc down a bit to create clear breathing room under the hub
    auto centre = discArea.getCentre();
    const float shiftDown = juce::jmax(8.0f, diameter * 0.08f);
    centre.setY(centre.y + shiftDown);

    discBounds = juce::Rectangle<float>(diameter, diameter).withCentre(centre);
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
    // Cmd+Shift+L toggles admin mode
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

void HALO9PlayerAudioProcessorEditor::updatePadButtonLabels()
{
    for (int i = 0; i < NUM_PADS; ++i)
    {
        if (processor.padSamplePaths[i].isNotEmpty())
            padButtons[i].setButtonText(
                juce::File(processor.padSamplePaths[i])
                    .getFileNameWithoutExtension().substring(0, 5));
    }
}

void HALO9PlayerAudioProcessorEditor::triggerPad(int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS) return;

    processor.getPadSampler().triggerPad(padIndex);
    padFlashEnd[padIndex] = juce::Time::getMillisecondCounterHiRes() + 120.0;
    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Status + Timer
// ═══════════════════════════════════════════════════════════════════════════════

void HALO9PlayerAudioProcessorEditor::setStatus(const juce::String& msg)
{
    statusLabel.setText(msg, juce::dontSendNotification);
}

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
