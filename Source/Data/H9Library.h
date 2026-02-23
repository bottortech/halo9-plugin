#pragma once
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

// ── Library data structures ──────────────────────────────────────────────────

struct H9PadInfo
{
    juce::String pad;       // "P1"–"P8"
    juce::String label;     // display name
    juce::String role;      // "synth", "fx", "texture", "sampler"
    juce::String slot;      // routing slot name
    juce::String file;      // sample file path (drum kits)
    float        gain { 1.0f };
};

struct H9PackData
{
    juce::String id;
    juce::String name;
    juce::String description;
    juce::Colour accentColor      { 0xff33ffc8 };
    juce::Colour keyHighlightColor{ 0xff33ffc8 };
    float        padGlowIntensity { 0.85f };
    juce::String badge;
    std::vector<H9PadInfo> padBank;
    juce::File   rootDir;
};

struct H9KitData
{
    juce::String id;
    juce::String name;
    juce::String description;
    juce::Colour accentColor      { 0xff33ffc8 };
    float        padGlowIntensity { 0.95f };
    juce::String badge;
    std::vector<H9PadInfo> pads;
    juce::File   rootDir;
};

// ── Library loader ───────────────────────────────────────────────────────────

class H9Library
{
public:
    bool loadFromDirectory(const juce::File& libraryRoot);

    const std::vector<H9PackData>& getPacks() const { return packs; }
    const std::vector<H9KitData>&  getKits()  const { return kits; }

    const H9PackData* findPack(const juce::String& id) const;
    const H9KitData*  findKit (const juce::String& id) const;

    juce::File getRoot()  const { return root; }
    bool       isLoaded() const { return loaded; }

    static juce::File findLibraryRoot();

private:
    juce::File root;
    bool loaded { false };
    std::vector<H9PackData> packs;
    std::vector<H9KitData>  kits;

    bool loadPackManifest(const juce::File& file, H9PackData& pack);
    bool loadKitManifest (const juce::File& file, H9KitData& kit);

    static juce::Colour parseColor(const juce::String& hex, juce::Colour fallback);
};
