/*
 * Copyright 2014 Intel Corporation All Rights Reserved. 
 * 
 * The source code contained or described herein and all documents related to the 
 * source code ("Material") are owned by Intel Corporation or its suppliers or 
 * licensors. Title to the Material remains with Intel Corporation or its suppliers 
 * and licensors. The Material contains trade secrets and proprietary and 
 * confidential information of Intel or its suppliers and licensors. The Material 
 * is protected by worldwide copyright and trade secret laws and treaty provisions. 
 * No part of the Material may be used, copied, reproduced, modified, published, 
 * uploaded, posted, transmitted, distributed, or disclosed in any way without 
 * Intel's prior express written permission.
 * 
 * No license under any patent, copyright, trade secret or other intellectual 
 * property right is granted to or conferred upon you by disclosure or delivery of 
 * the Materials, either expressly, by implication, inducement, estoppel or 
 * otherwise. Any license under such intellectual property rights must be express 
 * and approved by Intel in writing.
 */

#ifndef AudioMixer_h
#define AudioMixer_h

#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <logger.h>
#include <MediaDefinitions.h>
#include <MediaSourceConsumer.h>
#include <WoogeenTransport.h>
#include <webrtc/voice_engine/include/voe_base.h>
#include <webrtc/voice_engine/include/voe_video_sync.h>

namespace mcu {

class AudioMixer : public woogeen_base::MediaSourceConsumer, public erizo::MediaSink, public erizo::FeedbackSink {
    DECLARE_LOGGER();

public:
    AudioMixer(erizo::RTPDataReceiver*);
    virtual ~AudioMixer();

    // Implements the MediaSourceConsumer interfaces.
    int32_t addSource(uint32_t id, bool isAudio, erizo::FeedbackSink*);
    int32_t removeSource(uint32_t id, bool isAudio);
    erizo::MediaSink* mediaSink() { return this; }

    /**
     * Implements the MediaSink interfaces
     */
    int deliverAudioData(char*, int len);
    int deliverVideoData(char*, int len);

    // Implements the FeedbackSink interfaces
    int deliverFeedback(char* buf, int len);

    int32_t channelId(uint32_t sourceId);
    webrtc::VoEVideoSync* avSyncInterface();
    uint32_t sendSSRC();
    // TODO: Remove me once OOP Mixer is able to invoke addSource explicitly.
    void addSourceOnDemand(bool allow) { m_addSourceOnDemand = allow; }

private:
    int32_t performMix(const boost::system::error_code&);
    int32_t updateAudioFrame();

    struct VoiceChannel {
        int32_t id;
        boost::shared_ptr<woogeen_base::WoogeenTransport<erizo::AUDIO>> transport;
    };

    webrtc::VoiceEngine* m_voiceEngine;

    VoiceChannel m_outChannel;
    std::map<uint32_t, VoiceChannel> m_inChannels;
    boost::shared_mutex m_sourceMutex;

    boost::scoped_ptr<boost::thread> m_audioMixingThread;
    boost::asio::io_service m_ioService;
    boost::scoped_ptr<boost::asio::deadline_timer> m_timer;
    std::atomic<bool> m_isClosing;
    std::atomic<bool> m_addSourceOnDemand;
};

inline webrtc::VoEVideoSync* AudioMixer::avSyncInterface()
{
    return m_voiceEngine ? webrtc::VoEVideoSync::GetInterface(m_voiceEngine) : nullptr;
}

} /* namespace mcu */

#endif /* AudioMixer_h */
