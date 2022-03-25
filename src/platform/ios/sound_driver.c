/*

MIT License

Copyright (c) 2012 Peter Bjorklund

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include <clog/clog.h>
#include <stdbool.h>
#include <thunder/platform/ios/sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

#include <stdint.h>

const static int audio_buffer_size = 2 * 1024;

static void fill_buffer_callback(void* _self, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
    thunder_sound_driver* self = _self;
    thunder_audio_buffer* sound_buffer = self->buffer;

    buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
    if ((buffer->mAudioDataBytesCapacity % 4) != 0) {
        CLOG_WARN("ERROR!!!!!");
    }
    UInt32 sample_count = buffer->mAudioDataByteSize / sizeof(thunder_sample_output_s16);
    thunder_sample_output_s16* target = (thunder_sample_output_s16*) buffer->mAudioData;
    thunder_audio_buffer_read(sound_buffer, target, sample_count);

    int packet_count = 0;
    AudioStreamPacketDescription* packet_descriptions = 0;
    int enqueueResult = AudioQueueEnqueueBuffer(queue, buffer, packet_count, packet_descriptions);
    CLOG_ASSERT(enqueueResult == 0, "AudioQueueEnqueueBuffer failed")
}

static void create_and_fill_buffers(thunder_sound_driver* self)
{
    const int buffer_count = 4;
    AudioQueueBufferRef buffers[4];

    for (int i = 0; i < buffer_count; ++i) {
        int err = AudioQueueAllocateBuffer(self->zQueue, audio_buffer_size, &buffers[i]);
        CLOG_ASSERT(err == 0, "Couldn't allocate buffer")
        fill_buffer_callback(self, self->zQueue, buffers[i]);
    }
}

static void start_playback(thunder_sound_driver* self)
{
    UInt32 err = AudioQueueStart(self->zQueue, 0);
    CLOG_ASSERT(err == 0, "Audio queue start failed");
}

static void open_output(thunder_sound_driver* self, float sample_rate)
{
    AudioStreamBasicDescription zFormat;

    tc_mem_clear_type(&zFormat);

    zFormat.mSampleRate = sample_rate;
    zFormat.mFormatID = kAudioFormatLinearPCM;
    zFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    zFormat.mChannelsPerFrame = 2;
    zFormat.mFramesPerPacket = 1;
    zFormat.mBytesPerFrame = zFormat.mChannelsPerFrame * sizeof(thunder_sample_output_s16);
    zFormat.mBitsPerChannel = sizeof(thunder_sample_output_s16) * 8;
    zFormat.mBytesPerPacket = zFormat.mBytesPerFrame * zFormat.mFramesPerPacket;

    UInt32 err = AudioQueueNewOutput(&zFormat, fill_buffer_callback, self, 0, kCFRunLoopCommonModes, 0, &self->zQueue);
    CLOG_ASSERT(err == 0, "AudioQueueNewOutput err:%d", (unsigned int) err);
}

int thunder_sound_driver_init(thunder_sound_driver* self, thunder_audio_buffer* buffer)
{
    self->buffer = buffer;
    open_output(self, 48000.0f);
    create_and_fill_buffers(self);
    start_playback(self);

    return 0;
}

void thunder_sound_driver_free(thunder_sound_driver* self)
{
}
