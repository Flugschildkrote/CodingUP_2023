#include "anubis/sound/SoundFile.hpp"
#include <stdexcept>
#include <cassert>
#include <sndfile.hh>

namespace anubis
{
    AudioFile::AudioFile(const std::string& filePath)
    {
        SndfileHandle file(filePath.c_str(), SFM_READ);

        if (!file) {
            throw std::runtime_error("Failed to load sound \"" + filePath + "\"");
        }

        m_NumFrames = static_cast<uint32_t>(file.frames());
        m_NumChannels = static_cast<uint32_t>(file.channels());
        m_SampleRate = static_cast<uint32_t>(file.samplerate());

        uint32_t bufferSize = m_NumChannels * m_NumFrames;
        m_Samples = std::make_unique<double[]>(bufferSize);
        file.readf(m_Samples.get(), bufferSize);
    }

    AudioFile::ChannelView AudioFile::getChannel(size_t channelIndex) noexcept
    {
        assert(channelIndex < m_NumChannels && "channelIndex out of bounds");

        const size_t elementStride = sizeof(double) * m_NumChannels;
        const size_t bufferSize = sizeof(double) * m_NumChannels * m_NumFrames;
        const size_t elementOffset = sizeof(double) * channelIndex;
        return ChannelView(m_Samples.get(), bufferSize, elementOffset, elementStride);
    }

    AudioFile::ConstChannelView AudioFile::getChannel(size_t channelIndex) const noexcept
    {
        assert(channelIndex < m_NumChannels && "channelIndex out of bounds");

        const size_t elementStride = sizeof(double) * m_NumChannels;
        const size_t bufferSize = sizeof(double) * m_NumChannels * m_NumFrames;
        const size_t elementOffset = sizeof(double) * channelIndex;
        return ConstChannelView(m_Samples.get(), bufferSize, elementOffset, elementStride);
    }

    uint32_t AudioFile::getNumChanels(void) const noexcept
    {
        return m_NumChannels;
    }

    uint32_t AudioFile::getNumFrames(void) const noexcept
    {
        return m_NumFrames;
    }

    uint32_t AudioFile::getSampleRate(void) const noexcept
    {
        return m_NumChannels;
    }

    float AudioFile::getDuration(void) const noexcept
    {
        return static_cast<float>(m_NumFrames) / static_cast<float>(m_SampleRate);
    }

}