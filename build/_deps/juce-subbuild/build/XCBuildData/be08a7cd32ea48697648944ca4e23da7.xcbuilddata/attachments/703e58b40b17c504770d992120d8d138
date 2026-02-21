#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-src
  /usr/local/bin/cmake -Dcan_fetch=YES -DCMAKE_MESSAGE_LOG_LEVEL=VERBOSE -P /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-subbuild/juce-populate-prefix/tmp/juce-populate-gitupdate.cmake
fi

