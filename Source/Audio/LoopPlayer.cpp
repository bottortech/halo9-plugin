#include "LoopPlayer.h"

LoopPlayer::LoopPlayer()
{
    formatManager.registerBasicFormats();
    loadThread.startThread(juce::Thread::Priority::background);
}

LoopPlayer::~LoopPlayer()
{
    transport.stop();
    transport.setSource(nullptr);
    readerSource.reset();
    loadThread.stopThread(2000);
}

void LoopPlayer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    preparedSampleRate = sampleRate;
    preparedBlockSize  = samplesPerBlock;
    transport.prepareToPlay(samplesPerBlock, sampleRate);
    tempBuffer.setSize(2, samplesPerBlock);
}

void LoopPlayer::releaseResources()
{
    transport.stop();
    transport.releaseResources();
}

bool LoopPlayer::loadFile(const juce::File& file)
{
    const bool wasPlaying = transport.isPlaying();
    transport.stop();
    transport.setSource(nullptr);
    readerSource.reset();

    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return false;

    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    readerSource->setLooping(true);

    // 32768-sample read-ahead buffer on background thread prevents audio glitches
    transport.setSource(readerSource.get(), 32768, &loadThread, reader->sampleRate);
    transport.prepareToPlay(preparedBlockSize, preparedSampleRate);
    transport.setGain(volume.load());

    fileName   = file.getFileNameWithoutExtension();
    fileLoaded = true;

    if (wasPlaying)
        transport.start();

    return true;
}

void LoopPlayer::play()
{
    if (fileLoaded)
        transport.start();
}

void LoopPlayer::stop()
{
    transport.stop();
    transport.setPosition(0.0);
}

bool LoopPlayer::isPlaying() const
{
    return transport.isPlaying();
}

void LoopPlayer::setVolume(float vol)
{
    vol = juce::jlimit(0.0f, 1.0f, vol);
    volume.store(vol);
    transport.setGain(vol);
}

void LoopPlayer::processBlock(juce::AudioBuffer<float>& outputBuffer)
{
    if (!fileLoaded || !transport.isPlaying()) return;

    const int numSamples = outputBuffer.getNumSamples();
    const int numCh      = outputBuffer.getNumChannels();

    if (tempBuffer.getNumSamples() < numSamples || tempBuffer.getNumChannels() < numCh)
        tempBuffer.setSize(numCh, numSamples);

    tempBuffer.clear();
    juce::AudioSourceChannelInfo info (&tempBuffer, 0, numSamples);
    transport.getNextAudioBlock(info);

    for (int ch = 0; ch < numCh; ch++)
        outputBuffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);
}
