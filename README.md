# HALO9 Player — VST3 Plugin

A polished 8-pad sampler + loop player VST3, designed for FL Studio and any
VST3-compatible DAW on Windows and macOS.

---

## Features

| Feature | Details |
|---------|---------|
| 8-Pad sampler | One-shot playback, MIDI C1–G1 (notes 36–43), 8 voices |
| Loop player | Load any audio file, looping, with dedicated volume |
| Pack Browser | Scans `~/Documents/HALO9/Packs` for drum/loop libraries |
| Master Volume | Global output level |
| Lowpass Filter | 100 Hz – 20 kHz with warm log taper |
| Atmosphere macro | Drives reverb depth + stereo width + LPF tilt simultaneously |
| State save | Full DAW preset recall (pad paths, loop path, all knobs) |

---

## Pack Folder Structure

Place sample packs in:

```
~/Documents/HALO9/Packs/
  MyPack/
    Drums/
      Kick.wav
      Snare.wav
      Hat.wav
      ...
    Loops/
      FullLoop.wav
      TopLoop.wav
      ...
```

- If no `Drums/` subfolder exists, root audio files are treated as drum samples.
- If no `Loops/` subfolder exists, the loop list is empty for that pack.
- Supported formats: `.wav` `.mp3` `.aif` `.aiff` `.flac` `.ogg`

---

## Build — macOS

### Prerequisites
- Xcode 14+ (Command Line Tools)
- CMake 3.22+

### Steps

```bash
git clone https://github.com/juce-framework/JUCE.git ~/JUCE   # optional — or let FetchContent handle it
cd /path/to/HALO9_Plugin
cmake -B build -G Xcode
cmake --build build --config Release
```

The VST3 bundle is output to:
```
build/HALO9_Player_artefacts/Release/VST3/HALO9 Player.vst3
```

Copy it to your VST3 folder:
```bash
cp -r "build/HALO9_Player_artefacts/Release/VST3/HALO9 Player.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/
```

### With a local JUCE checkout (skip FetchContent)

```bash
cmake -B build -G Xcode -DJUCE_DIR=/path/to/JUCE
```

---

## Build — Windows

### Prerequisites
- Visual Studio 2022 (Desktop C++ workload)
- CMake 3.22+

### Steps (PowerShell)

```powershell
git clone https://github.com/juce-framework/JUCE.git C:\JUCE   # optional
cd C:\path\to\HALO9_Plugin
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The VST3 bundle is output to:
```
build\HALO9_Player_artefacts\Release\VST3\HALO9 Player.vst3
```

Copy it to your DAW's VST3 scan folder (e.g.):
```
C:\Program Files\Common Files\VST3\
```

### With a local JUCE checkout

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=C:\JUCE
```

---

## MIDI Map

| Pad | MIDI Note | Default Key |
|-----|-----------|-------------|
| 1   | C1 (36)   | — |
| 2   | C#1 (37)  | — |
| 3   | D1 (38)   | — |
| 4   | D#1 (39)  | — |
| 5   | E1 (40)   | — |
| 6   | F1 (41)   | — |
| 7   | F#1 (42)  | — |
| 8   | G1 (43)   | — |

---

## Parameters

| ID | Range | Default | Description |
|----|-------|---------|-------------|
| `master_volume` | 0 – 1 | 0.7 | Output gain |
| `lowpass_cutoff` | 100 – 20000 Hz | 20000 | Lowpass filter frequency |
| `atmosphere` | 0 – 1 | 0.0 | Reverb + width + LPF tilt macro |
| `loop_volume` | 0 – 1 | 0.8 | Loop player level |

All parameters are automatable in the DAW.

---

## Signal Chain

```
[PadSampler]  ──┐
                 ├──► M/S Width ──► [LPF] ──► [Reverb] ──► [Master Gain]
[LoopPlayer]  ──┘
```

The Atmosphere knob drives:
- Reverb room size 0 → 0.85
- Reverb wet level 0 → 0.45
- Stereo width ×1.0 → ×2.2
- LPF cutoff reduction (up to −50% at maximum)
