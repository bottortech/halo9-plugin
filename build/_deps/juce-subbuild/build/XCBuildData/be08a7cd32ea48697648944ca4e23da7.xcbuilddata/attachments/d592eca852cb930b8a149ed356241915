#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-build
  /usr/local/bin/cmake -E echo_append
  /usr/local/bin/cmake -E touch /Users/owner/Documents/HALO9_Plugin/build/_deps/juce-subbuild/juce-populate-prefix/src/juce-populate-stamp/$CONFIGURATION$EFFECTIVE_PLATFORM_NAME/juce-populate-build
fi

