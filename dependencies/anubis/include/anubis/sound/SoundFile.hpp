#ifndef ANUBIS_SOUND_HPP
#define ANUBIS_SOUND_HPP

#include <string>
#include <memory>
#include <anubis/util/MemoryView.hpp>

namespace anubis {


class AudioFile
{
    using ChannelView = anubis::MemoryView<double>;
    using ConstChannelView = anubis::MemoryView<const double>;
public:

    AudioFile(const std::string& filePath);
    AudioFile(const AudioFile&) = delete;
    AudioFile(AudioFile&&) = default;

    AudioFile& operator=(const AudioFile&) = delete;
    AudioFile& operator=(AudioFile&&) = default;

    ChannelView getChannel(size_t channelIndex) noexcept;
    ConstChannelView getChannel(size_t channelIndex) const noexcept;
    uint32_t getNumChanels(void) const noexcept;
    uint32_t getNumFrames(void) const noexcept;
    uint32_t getSampleRate(void) const noexcept;

    /**
     * @brief Get the duration of the sound in seconds
     */
    float getDuration(void) const noexcept;
private:

    uint32_t m_NumFrames;
    uint32_t m_NumChannels;
    uint32_t m_SampleRate;
    std::unique_ptr<double[]> m_Samples;
};

}

#endif