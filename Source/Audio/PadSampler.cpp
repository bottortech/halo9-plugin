#include "PadSampler.h"

PadSampler::PadSampler()
{
    formatManager.registerBasicFormats();
}

void PadSampler::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    for (auto& v : voices)
        v.position = -1;
}

// ── Sample loading ─────────────────────────────────────────────────────────────

bool PadSampler::loadSample(int padIndex, const juce::File& file)
{
    if (padIndex < 0 || padIndex >= NUM_PADS) return false;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
    if (reader == nullptr) return false;

    auto& pad = pads[padIndex];
    pad.buffer.setSize((int)reader->numChannels,
                       (int)juce::jmin((juce::int64)reader->lengthInSamples,
                                       (juce::int64)reader->sampleRate * 30)); // cap 30s
    reader->read(&pad.buffer, 0, pad.buffer.getNumSamples(), 0, true, true);
    pad.fileName = file.getFileNameWithoutExtension();
    pad.hasSound = true;
    return true;
}

void PadSampler::clearSample(int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS) return;
    pads[padIndex].hasSound = false;
    pads[padIndex].fileName.clear();
}

juce::String PadSampler::getSampleName(int padIndex) const
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
        return pads[padIndex].fileName;
    return {};
}

bool PadSampler::hasSample(int padIndex) const
{
    if (padIndex >= 0 && padIndex < NUM_PADS)
        return pads[padIndex].hasSound;
    return false;
}

// ── Trigger ────────────────────────────────────────────────────────────────────

void PadSampler::triggerPad(int padIndex, float velocity)
{
    if (padIndex < 0 || padIndex >= NUM_PADS) return;
    if (!pads[padIndex].hasSound)             return;
    startVoice(pads[padIndex].buffer, velocity);
}

void PadSampler::startVoice(const juce::AudioBuffer<float>& sample, float velocity)
{
    auto& v    = voices[nextVoice % NUM_VOICES];
    v.srcBuffer = &sample;
    v.position  = 0;
    v.gain      = juce::jlimit(0.0f, 1.0f, velocity);
    nextVoice   = (nextVoice + 1) % NUM_VOICES;
}

// ── processBlock ──────────────────────────────────────────────────────────────

void PadSampler::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    // Handle incoming MIDI note-on messages
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int note   = msg.getNoteNumber();
            const int padIdx = note - MIDI_PAD_START;
            if (padIdx >= 0 && padIdx < NUM_PADS)
                triggerPad(padIdx, msg.getFloatVelocity());
        }
    }

    const int numOut     = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    for (auto& voice : voices)
    {
        if (!voice.isPlaying()) continue;

        const int available = voice.srcBuffer->getNumSamples() - voice.position;
        const int toRender  = juce::jmin(available, numOut);

        for (int ch = 0; ch < numCh; ch++)
        {
            const int srcCh = juce::jmin(ch, voice.srcBuffer->getNumChannels() - 1);
            buffer.addFrom(ch, 0,
                           *voice.srcBuffer, srcCh, voice.position,
                           toRender, voice.gain);
        }

        voice.position += toRender;
        if (voice.position >= voice.srcBuffer->getNumSamples())
        {
            voice.position  = -1;
            voice.srcBuffer = nullptr;
        }
    }
}
