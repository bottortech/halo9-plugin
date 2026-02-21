#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E copy /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_Standalone/PkgInfo "/Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Debug/Standalone/HALO9 Player.app/Contents"
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E make_directory /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Debug/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding
  /usr/local/bin/cmake -E copy_if_different /Users/owner/Documents/HALO9_Plugin/assets/branding/halo9.png /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Debug/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding/halo9.png
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E copy /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_Standalone/PkgInfo "/Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Release/Standalone/HALO9 Player.app/Contents"
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E make_directory /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Release/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding
  /usr/local/bin/cmake -E copy_if_different /Users/owner/Documents/HALO9_Plugin/assets/branding/halo9.png /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/Release/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding/halo9.png
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E copy /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_Standalone/PkgInfo "/Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/MinSizeRel/Standalone/HALO9 Player.app/Contents"
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E make_directory /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/MinSizeRel/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding
  /usr/local/bin/cmake -E copy_if_different /Users/owner/Documents/HALO9_Plugin/assets/branding/halo9.png /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/MinSizeRel/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding/halo9.png
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E copy /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/JuceLibraryCode/HALO9_Player_Standalone/PkgInfo "/Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/RelWithDebInfo/Standalone/HALO9 Player.app/Contents"
  cd /Users/owner/Documents/HALO9_Plugin/build
  /usr/local/bin/cmake -E make_directory /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/RelWithDebInfo/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding
  /usr/local/bin/cmake -E copy_if_different /Users/owner/Documents/HALO9_Plugin/assets/branding/halo9.png /Users/owner/Documents/HALO9_Plugin/build/HALO9_Player_artefacts/RelWithDebInfo/Standalone/HALO9\ Player.app/Contents/MacOS/../Resources/assets/branding/halo9.png
fi

