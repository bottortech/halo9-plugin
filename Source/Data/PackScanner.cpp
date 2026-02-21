#include "PackScanner.h"

juce::File PackScanner::getPacksDirectory()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("HALO9")
                   .getChildFile("Packs");
    if (!dir.exists())
        dir.createDirectory();
    return dir;
}

juce::Array<juce::File> PackScanner::scanAudioFiles(const juce::File& dir)
{
    juce::Array<juce::File> result;
    if (!dir.exists()) return result;

    static const juce::StringArray exts { ".wav", ".mp3", ".aif", ".aiff", ".flac", ".ogg" };

    for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*", juce::File::findFiles))
    {
        const auto& f = entry.getFile();
        if (exts.contains(f.getFileExtension().toLowerCase()))
            result.add(f);
    }

    // Natural sort by filename
    std::sort(result.begin(), result.end(),
        [](const juce::File& a, const juce::File& b)
        {
            return a.getFileName().compareNatural(b.getFileName()) < 0;
        });

    return result;
}

void PackScanner::scan()
{
    packs.clear();
    auto packsDir = getPacksDirectory();

    for (const auto& entry : juce::RangedDirectoryIterator(packsDir, false, "*", juce::File::findDirectories))
    {
        const auto& packDir = entry.getFile();
        Pack pack;
        pack.name      = packDir.getFileName();
        pack.directory = packDir;

        // Prefer Drums/ and Loops/ sub-folders; fall back to root audio files
        auto drumsDir = packDir.getChildFile("Drums");
        auto loopsDir = packDir.getChildFile("Loops");

        pack.drumFiles = drumsDir.exists()
                         ? scanAudioFiles(drumsDir)
                         : scanAudioFiles(packDir);   // fallback: root files as drums

        pack.loopFiles = scanAudioFiles(loopsDir);

        if (!pack.drumFiles.isEmpty() || !pack.loopFiles.isEmpty())
            packs.add(pack);
    }

    // Sort packs alphabetically
    std::sort(packs.begin(), packs.end(),
        [](const Pack& a, const Pack& b)
        {
            return a.name.compareNatural(b.name) < 0;
        });
}
