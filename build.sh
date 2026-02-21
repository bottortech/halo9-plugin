#!/usr/bin/env bash
# ── HALO9 Player — Build Script ──────────────────────────────────────────────
#
# Usage:
#   ./build.sh                  Build Release (Xcode on macOS, Makefiles on Linux)
#   ./build.sh debug            Build Debug
#   ./build.sh clean            Remove build dir and start fresh
#   ./build.sh xcode            Generate Xcode project only (don't build)
#   ./build.sh install          Build Release + install VST3 to system folder
#   JUCE_DIR=/path/to/JUCE ./build.sh   Use local JUCE instead of FetchContent
#
# Notes:
#   On macOS the build dir is placed in /tmp to avoid iCloud codesign issues
#   (resource fork xattrs on ~/Documents cause `codesign` to fail).
#   Artifacts are symlinked back to the project for easy access.
#
# Requirements:
#   macOS  — Xcode 14+, CMake 3.22+
#   Linux  — GCC 10+ or Clang 13+, CMake 3.22+, ALSA dev headers
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG="${1:-release}"

# Build directory: use project-local build/
# Note: If you encounter iCloud codesign xattr issues on macOS, either:
#   a) Move the app to /tmp after building, or
#   b) Exclude ~/Documents from iCloud in System Preferences
BUILD_DIR="${PROJECT_DIR}/build"

# ── Helpers ──────────────────────────────────────────────────────────────────

red()   { printf '\033[1;31m%s\033[0m\n' "$*"; }
green() { printf '\033[1;32m%s\033[0m\n' "$*"; }
cyan()  { printf '\033[1;36m%s\033[0m\n' "$*"; }

die() { red "ERROR: $*" >&2; exit 1; }

check_tool() {
    command -v "$1" >/dev/null 2>&1 || die "$1 not found. Please install it first."
}

# ── Ensure Homebrew tools are on PATH (macOS) ──────────────────────────────

if [ "$(uname)" = "Darwin" ]; then
    for p in /usr/local/bin /opt/homebrew/bin; do
        case ":${PATH}:" in
            *":${p}:"*) ;;
            *) [ -d "$p" ] && export PATH="${p}:${PATH}" ;;
        esac
    done
fi

# ── Prerequisite checks ─────────────────────────────────────────────────────

check_tool cmake

CMAKE_VER=$(cmake --version | head -1 | grep -oE '[0-9]+\.[0-9]+')
if [ "$(printf '%s\n' "3.22" "$CMAKE_VER" | sort -V | head -1)" != "3.22" ]; then
    die "CMake 3.22+ required (found ${CMAKE_VER})"
fi

# ── Handle "clean" command ───────────────────────────────────────────────────

if [ "$CONFIG" = "clean" ]; then
    cyan "Removing ${BUILD_DIR}..."
    rm -rf "${BUILD_DIR}"
    green "Clean complete."
    exit 0
fi

# ── Determine generator and build config ─────────────────────────────────────

if [ "$(uname)" = "Darwin" ]; then
    check_tool xcodebuild
    GENERATOR="Xcode"
    if [ "$CONFIG" = "debug" ]; then
        BUILD_CONFIG="Debug"
    else
        BUILD_CONFIG="Release"
    fi
else
    if command -v ninja >/dev/null 2>&1; then
        GENERATOR="Ninja"
    else
        GENERATOR="Unix Makefiles"
    fi
    if [ "$CONFIG" = "debug" ]; then
        BUILD_CONFIG="Debug"
    else
        BUILD_CONFIG="Release"
    fi
fi

# ── Optional: pass local JUCE path ──────────────────────────────────────────

CMAKE_EXTRA_ARGS=()
if [ -n "${JUCE_DIR:-}" ]; then
    cyan "Using local JUCE: ${JUCE_DIR}"
    CMAKE_EXTRA_ARGS+=("-DJUCE_DIR=${JUCE_DIR}")
fi

# ── "xcode" command: generate only, don't build ─────────────────────────────

if [ "$CONFIG" = "xcode" ]; then
    if [ "$(uname)" != "Darwin" ]; then
        die "Xcode generator only available on macOS"
    fi
    cyan "Generating Xcode project..."
    cmake -B "${BUILD_DIR}" -G Xcode "${CMAKE_EXTRA_ARGS[@]+"${CMAKE_EXTRA_ARGS[@]}"}" "${PROJECT_DIR}"


    green "Xcode project: ${BUILD_DIR}/HALO9_Player.xcodeproj"
    green "Open with: open ${PROJECT_DIR}/build/HALO9_Player.xcodeproj"
    exit 0
fi

# ── Configure ────────────────────────────────────────────────────────────────

cyan "Configuring (${GENERATOR}, ${BUILD_CONFIG})..."

if [ "$GENERATOR" = "Xcode" ]; then
    cmake -B "${BUILD_DIR}" \
          -G "${GENERATOR}" \
          "${CMAKE_EXTRA_ARGS[@]+"${CMAKE_EXTRA_ARGS[@]}"}" \
          "${PROJECT_DIR}"
else
    cmake -B "${BUILD_DIR}" \
          -G "${GENERATOR}" \
          -DCMAKE_BUILD_TYPE="${BUILD_CONFIG}" \
          "${CMAKE_EXTRA_ARGS[@]+"${CMAKE_EXTRA_ARGS[@]}"}" \
          "${PROJECT_DIR}"
fi

# ── Build ────────────────────────────────────────────────────────────────────

cyan "Building ${BUILD_CONFIG}..."
cmake --build "${BUILD_DIR}" --config "${BUILD_CONFIG}" --parallel

# ── Report output paths ─────────────────────────────────────────────────────

ARTEFACTS="${BUILD_DIR}/HALO9_Player_artefacts/${BUILD_CONFIG}"

green ""
green "BUILD SUCCESSFUL"
green ""

if [ -d "${ARTEFACTS}/VST3" ]; then
    echo "  VST3:       ${ARTEFACTS}/VST3/HALO9 Player.vst3"
fi

if [ -d "${ARTEFACTS}/Standalone" ]; then
    echo "  Standalone: ${ARTEFACTS}/Standalone/HALO9 Player.app"
fi

echo ""

# ── "install" command: copy VST3 to system plugin folder ─────────────────────

if [ "$CONFIG" = "install" ]; then
    if [ "$(uname)" = "Darwin" ]; then
        VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3"
    else
        VST3_DEST="$HOME/.vst3"
    fi

    mkdir -p "${VST3_DEST}"
    cp -r "${ARTEFACTS}/VST3/HALO9 Player.vst3" "${VST3_DEST}/"
    green "Installed: ${VST3_DEST}/HALO9 Player.vst3"
else
    cyan "Install VST3:"
    if [ "$(uname)" = "Darwin" ]; then
        echo "  cp -r \"${ARTEFACTS}/VST3/HALO9 Player.vst3\" ~/Library/Audio/Plug-Ins/VST3/"
    else
        echo "  cp -r \"${ARTEFACTS}/VST3/HALO9 Player.vst3\" ~/.vst3/"
    fi
fi
