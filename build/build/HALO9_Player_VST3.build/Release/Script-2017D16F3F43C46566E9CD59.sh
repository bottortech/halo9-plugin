#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-build/tools/extras/Build/juceaide/juceaide_artefacts/Debug/juceaide pkginfo VST3 /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_VST3/PkgInfo
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-build/tools/extras/Build/juceaide/juceaide_artefacts/Debug/juceaide pkginfo VST3 /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_VST3/PkgInfo
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-build/tools/extras/Build/juceaide/juceaide_artefacts/Debug/juceaide pkginfo VST3 /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_VST3/PkgInfo
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-build/tools/extras/Build/juceaide/juceaide_artefacts/Debug/juceaide pkginfo VST3 /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_VST3/PkgInfo
fi

