#include "H9Library.h"

// ── Library root discovery ───────────────────────────────────────────────────

juce::File H9Library::findLibraryRoot()
{
    // 1. Environment variable override (admin/dev)
    auto envPath = juce::SystemStats::getEnvironmentVariable("HALO9_LIBRARY_PATH", "");
    if (envPath.isNotEmpty())
    {
        juce::File f(envPath);
        if (f.getChildFile("library.json").existsAsFile())
            return f;
    }

    // 2. User application data (~/Library/Application Support/HALO9/library)
    auto userLib = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("HALO9/library");
    if (userLib.getChildFile("library.json").existsAsFile())
        return userLib;

    // 3. Development path (set by CMake compile definition)
#ifdef HALO9_DEV_LIBRARY_PATH
    {
        juce::File devLib(HALO9_DEV_LIBRARY_PATH);
        if (devLib.getChildFile("library.json").existsAsFile())
            return devLib;
    }
#endif

    // 4. App bundle resources (production)
    auto app = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    auto res = app.getChildFile("Contents/Resources/halo9_library");
    if (res.getChildFile("library.json").existsAsFile())
        return res;

    return {};
}

// ── Main loader ──────────────────────────────────────────────────────────────

bool H9Library::loadFromDirectory(const juce::File& libraryRoot)
{
    root = libraryRoot;
    packs.clear();
    kits.clear();
    loaded = false;

    auto indexFile = root.getChildFile("library.json");
    if (!indexFile.existsAsFile()) return false;

    auto parsed = juce::JSON::parse(indexFile.loadFileAsString());
    if (!parsed.isObject()) return false;

    // ── Parse packs ──────────────────────────────────────────────────────
    auto packsArr = parsed["packs"];
    if (packsArr.isArray())
    {
        for (int i = 0; i < packsArr.size(); ++i)
        {
            auto entry = packsArr[i];
            auto path = entry["path"].toString();
            auto mf = root.getChildFile(path).getChildFile("manifest.json");

            if (mf.existsAsFile())
            {
                H9PackData pack;
                pack.id   = entry["id"].toString();
                pack.name = entry["name"].toString();
                if (loadPackManifest(mf, pack))
                    packs.push_back(std::move(pack));
            }
        }
    }

    // ── Parse drum kits ──────────────────────────────────────────────────
    auto kitsArr = parsed["drumkits"];
    if (kitsArr.isArray())
    {
        for (int i = 0; i < kitsArr.size(); ++i)
        {
            auto entry = kitsArr[i];
            auto path = entry["path"].toString();
            auto mf = root.getChildFile(path).getChildFile("manifest.json");

            if (mf.existsAsFile())
            {
                H9KitData kit;
                kit.id   = entry["id"].toString();
                kit.name = entry["name"].toString();
                if (loadKitManifest(mf, kit))
                    kits.push_back(std::move(kit));
            }
        }
    }

    loaded = !packs.empty() || !kits.empty();
    return loaded;
}

// ── Pack manifest ────────────────────────────────────────────────────────────

bool H9Library::loadPackManifest(const juce::File& file, H9PackData& pack)
{
    auto m = juce::JSON::parse(file.loadFileAsString());
    if (!m.isObject()) return false;

    pack.rootDir     = file.getParentDirectory();
    pack.description = m["description"].toString();
    pack.accentColor = parseColor(m["accentColor"].toString(), juce::Colour(0xff33ffc8));

    auto ui = m["ui"];
    if (ui.isObject())
    {
        pack.badge             = ui["badge"].toString();
        pack.keyHighlightColor = parseColor(ui["keyboardHighlightColor"].toString(),
                                            pack.accentColor);
        auto glow = ui["padGlowIntensity"];
        if (!glow.isVoid())
            pack.padGlowIntensity = static_cast<float>(static_cast<double>(glow));
    }

    auto padBank = m["padBank"];
    if (padBank.isArray())
    {
        for (int i = 0; i < padBank.size(); ++i)
        {
            auto p = padBank[i];
            H9PadInfo info;
            info.pad   = p["pad"].toString();
            info.label = p["label"].toString();
            info.role  = p["role"].toString();
            info.slot  = p["slot"].toString();
            pack.padBank.push_back(info);
        }
    }

    return true;
}

// ── Kit manifest ─────────────────────────────────────────────────────────────

bool H9Library::loadKitManifest(const juce::File& file, H9KitData& kit)
{
    auto m = juce::JSON::parse(file.loadFileAsString());
    if (!m.isObject()) return false;

    kit.rootDir     = file.getParentDirectory();
    kit.description = m["description"].toString();
    kit.accentColor = parseColor(m["accentColor"].toString(), juce::Colour(0xff33ffc8));

    auto ui = m["ui"];
    if (ui.isObject())
    {
        kit.badge = ui["badge"].toString();
        auto glow = ui["padGlowIntensity"];
        if (!glow.isVoid())
            kit.padGlowIntensity = static_cast<float>(static_cast<double>(glow));
    }

    auto mapping = m["mapping"];
    if (mapping.isObject())
    {
        auto padsArr = mapping["pads"];
        if (padsArr.isArray())
        {
            for (int i = 0; i < padsArr.size(); ++i)
            {
                auto p = padsArr[i];
                H9PadInfo info;
                info.pad   = p["pad"].toString();
                info.label = p["name"].toString();
                info.file  = p["file"].toString();
                auto g = p["gain"];
                info.gain  = g.isVoid() ? 1.0f : static_cast<float>(static_cast<double>(g));
                kit.pads.push_back(info);
            }
        }
    }

    return true;
}

// ── Lookup ───────────────────────────────────────────────────────────────────

const H9PackData* H9Library::findPack(const juce::String& id) const
{
    for (auto& p : packs)
        if (p.id == id) return &p;
    return nullptr;
}

const H9KitData* H9Library::findKit(const juce::String& id) const
{
    for (auto& k : kits)
        if (k.id == id) return &k;
    return nullptr;
}

// ── Color parsing (#RRGGBB or #AARRGGBB) ────────────────────────────────────

juce::Colour H9Library::parseColor(const juce::String& hex, juce::Colour fallback)
{
    if (hex.isEmpty()) return fallback;
    auto clean = hex.trimCharactersAtStart("#");

    if (clean.length() == 6)
        return juce::Colour(static_cast<juce::uint32>(clean.getHexValue32()) | 0xff000000u);
    if (clean.length() == 8)
        return juce::Colour(static_cast<juce::uint32>(clean.getHexValue32()));

    return fallback;
}
