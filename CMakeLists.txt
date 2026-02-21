cmake_minimum_required(VERSION 3.22)
project(HALO9_Player VERSION 1.0.0 LANGUAGES CXX)

# ── C++ standard ─────────────────────────────────────────────────────────────
# JUCE 7 requires C++17 minimum. REQUIRED + no extensions keeps builds strict.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── macOS deployment target ──────────────────────────────────────────────────
# 10.15 (Catalina) is the minimum for JUCE 7 with C++17 filesystem support.
# Must be set BEFORE any target is created (including JUCE's add_subdirectory).
if(APPLE)
    # Force-set (not CACHE) so Xcode respects our values over SDK defaults.
    # 10.15 (Catalina) = minimum for JUCE 7 + C++17 filesystem.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
    set(CMAKE_OSX_ARCHITECTURES    "x86_64;arm64")

    # Mirror as Xcode build attribute — guarantees the Xcode project
    # picks this up even when the CMake variable alone doesn't propagate.
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.15")
endif()

# ── Compile-commands export ──────────────────────────────────────────────────
# Generates compile_commands.json in build/ — VSCode/clangd uses this to
# resolve JUCE module includes and kill the red squiggles.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── JUCE dependency ──────────────────────────────────────────────────────────
# Two ways to provide JUCE:
#   A) Automatic: FetchContent downloads JUCE 7.0.12 at configure time.
#   B) Manual:    cmake -DJUCE_DIR=/path/to/JUCE -B build
# Option B avoids re-downloading on every clean build.
if(DEFINED JUCE_DIR)
    # Use a local JUCE checkout — faster iteration, no network required.
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE          # only fetch the tagged commit (fast)
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ── Plugin target ────────────────────────────────────────────────────────────
# juce_add_plugin creates per-format targets:
#   HALO9_Player_VST3       → .vst3 bundle
#   HALO9_Player_Standalone → .app (runnable in Xcode)
# Add AU here if you later need Logic/GarageBand support.
juce_add_plugin(HALO9_Player
    COMPANY_NAME                "HALO9"
    BUNDLE_ID                   "com.halo9.player"
    PLUGIN_MANUFACTURER_CODE    H9Co
    PLUGIN_CODE                 H9PL
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "HALO9 Player"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Instrument" "Synth"
)

# Generate JuceHeader.h with all JucePlugin_* macros (JucePlugin_Name, etc.)
juce_generate_juce_header(HALO9_Player)

# ── Source files ─────────────────────────────────────────────────────────────
target_sources(HALO9_Player PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/Audio/PadSampler.h
    Source/Audio/PadSampler.cpp
    Source/Audio/H9SynthVoice.h
    Source/Data/PackScanner.h
    Source/Data/PackScanner.cpp
    Source/Data/FactorySounds.h
    Source/Data/H9Library.h
    Source/Data/H9Library.cpp
    Source/UI/H9LookAndFeel.h
    Source/UI/H9LookAndFeel.cpp
    Source/EmbeddedHalo9.cpp
)

# ── Include paths ────────────────────────────────────────────────────────────
# Source files use relative includes like "Audio/PadSampler.h" and
# "Data/PackScanner.h". CMake doesn't auto-add source directories to the
# include path, so we must do it explicitly. Without this, Makefiles and
# Ninja generators fail even if Xcode happens to work.
target_include_directories(HALO9_Player PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# ── Compile definitions ─────────────────────────────────────────────────────
# These silence unused JUCE subsystems and prevent linker warnings.
target_compile_definitions(HALO9_Player
    PUBLIC
        JUCE_WEB_BROWSER=0                  # no embedded browser needed
        JUCE_USE_CURL=0                     # no HTTP/CURL needed
        JUCE_VST3_CAN_REPLACE_VST2=0       # no VST2 compatibility
        JUCE_DISPLAY_SPLASH_SCREEN=0        # suppress GPL splash screen
        JUCE_STRICT_REFCOUNTEDPOINTER=1     # catch ref-count bugs at compile time
    PRIVATE
        HALO9_DEV_LIBRARY_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/halo9_library"
)

# ── Link JUCE modules ───────────────────────────────────────────────────────
# PRIVATE: only this target needs the module symbols.
# PUBLIC juce::juce_recommended_*: propagates recommended flags to dependents.
target_link_libraries(HALO9_Player
    PRIVATE
        juce::juce_audio_utils          # AudioProcessorEditor, FileChooser
        juce::juce_audio_plugin_client  # VST3/Standalone entry points
        juce::juce_dsp                  # IIR, Reverb, Gain, ProcessorChain
        juce::juce_gui_basics           # Component, ListBox, Slider, Label
        juce::juce_audio_formats        # AudioFormatManager, WAV/MP3 codecs
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Copy branding assets into Standalone app bundle Resources so runtime
#     can load them from Contents/Resources/assets/branding/halo9.png
if(APPLE)
    add_custom_command(TARGET HALO9_Player_Standalone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:HALO9_Player_Standalone>/../Resources/assets/branding"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/branding/halo9.png"
            "$<TARGET_FILE_DIR:HALO9_Player_Standalone>/../Resources/assets/branding/halo9.png"
    )
endif()
