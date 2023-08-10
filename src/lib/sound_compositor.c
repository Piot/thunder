/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <imprint/allocator.h>
#include <thunder/audio_node.h>
#include <thunder-audio-buffer/sound_buffer.h>
#include <thunder/sound_compositor.h>

#define THUNDER_ATOM_SAMPLE_COUNT (3200)
#define THUNDER_ATOM_COUNT_BUFFER (4)

static void mixDownUsingVolume(ThunderMixSample* source, size_t size, float mix_down_volume, ThunderSample* target)
{
    const size_t DIVISOR = 8;
    int64_t mix = (int64_t) (mix_down_volume * 256);
    CLOG_ASSERT((THUNDER_ATOM_SAMPLE_COUNT % DIVISOR) == 0, "Illegal divisor")
    for (size_t i = 0; i < size / DIVISOR; ++i) {
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
        *target++ = (ThunderSample) ((*source++ * mix) >> 8);
    }
}

static ThunderMixSample maximumAmplitude(const ThunderMixSample* target, int size)
{
    ThunderMixSample maximum_amplitude_found = 0;
    ThunderMixSample minimum_amplitude_found = 0;
    ThunderMixSample v;

    for (int i = 0; i < size; ++i) {
        v = *target++;

        if (v > maximum_amplitude_found) {
            maximum_amplitude_found = v;
        } else if (v < minimum_amplitude_found) {
            minimum_amplitude_found = v;
        }
    }

    ThunderMixSample max_amplitude = (-minimum_amplitude_found > maximum_amplitude_found) ? -minimum_amplitude_found
                                                                                          : maximum_amplitude_found;

    return max_amplitude;
}

static void adjustMixDownVolume(ThunderAudioCompositor* self, ThunderMixSample maximum_amplitude_found)
{
    float maximum_amplitude_as_factor = (float) maximum_amplitude_found / 32767.0f;
    if (maximum_amplitude_found <= 2) {
        return;
    }
    float optimal_volume = (1.0f / maximum_amplitude_as_factor);
    // We don't want to increase the volume too much
    const static float max_volume = 1.4f;
    if (optimal_volume > max_volume) {
        optimal_volume = max_volume;
    }
    if (optimal_volume < self->mixDownVolume) {
        self->mixDownVolume = optimal_volume * 0.95f;
        // CLOG_VERBOSE("HIGH AMPLITUDE ADJUSTING: Max Found:%d, Dynamics:%f", maximum_amplitude_found,
        // self->mix_down_volume);
    } else if (optimal_volume >=
               self->mixDownVolume + 0.1f) { // It must be a big difference to be bother to increased the volume
        self->mixDownVolume += (optimal_volume - self->mixDownVolume) / (float) (60 * 30);
        // CLOG_VERBOSE("slightly increasing Max Found:%d, Dynamics:%f", maximum_amplitude_found,
        // self->mix_down_volume);
    }
}

static void addToOutput(ThunderMixSample* output, int channel, int number_of_channels,
                          const ThunderSample* source_sample, float volume)
{
    ThunderMixSample* target = output + channel;
    ThunderMixSample* end = output + THUNDER_ATOM_SAMPLE_COUNT; // - channel;
    const ThunderSample* source = source_sample;
    ThunderMixSample factor = (ThunderMixSample) (volume * 256);

    while (target < end) {
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
        *target += (*source++ * factor) >> 8;
        target += number_of_channels;
    }
}

static void enumerateNodes(ThunderAudioCompositor* self)
{
    ThunderSample source[THUNDER_ATOM_SAMPLE_COUNT];

    static const int LEFT_CHANNEL = 0;
    static const int RIGHT_CHANNEL = 1;
    static const int NUMBER_OF_OUTPUT_CHANNELS = 2;

    ThunderMixSample* output = self->output;

    for (size_t i = 0; i < self->nodesCount; ++i) {
        ThunderAudioNode* node = &self->nodes[i];

        if (node->is_playing) {
            if (node->channel_count == 2) {
                node->output(node->_self, source, THUNDER_ATOM_SAMPLE_COUNT / 2);
                addToOutput(output, 0, 1, source, node->volume);
            } else {
                node->output(node->_self, source, THUNDER_ATOM_SAMPLE_COUNT / 2);
                float normalized_pan = ((float) node->pan + 1.0f) / 2.0f;
                addToOutput(output, LEFT_CHANNEL, NUMBER_OF_OUTPUT_CHANNELS, source, normalized_pan * node->volume);
                addToOutput(output, RIGHT_CHANNEL, NUMBER_OF_OUTPUT_CHANNELS, source,
                            (1.0f - normalized_pan) * node->volume);
            }
        }
    }
}

static void compressTo16Bit(ThunderAudioCompositor* self, ThunderSample* output)
{
    ThunderMixSample max_amplitude = maximumAmplitude(self->output, THUNDER_ATOM_SAMPLE_COUNT);

    adjustMixDownVolume(self, max_amplitude);
    self->mixDownVolume = 1.0f;
    mixDownUsingVolume(self->output, THUNDER_ATOM_SAMPLE_COUNT, self->mixDownVolume, output);
}

static void clearBuffer(ThunderMixSample* output)
{
    tc_mem_clear_type_n(output, THUNDER_ATOM_SAMPLE_COUNT);
}

void thunderAudioCompositorUpdate(ThunderAudioCompositor* self)
{
    clearBuffer(self->output);
    enumerateNodes(self);
    compressTo16Bit(self, self->output16Bit);
    thunderAudioBufferWrite(&self->buffer, self->output16Bit, THUNDER_ATOM_SAMPLE_COUNT);
}

void thunderAudioCompositorReload(ThunderAudioCompositor* self)
{
    for (size_t i = 0; i < self->nodesMaxCount; ++i) {
        ThunderAudioNode* node = &self->nodes[i];
        node->is_playing = 0;
        node->output = 0;
    }

    self->nodesCount = 0;
}

void thunderAudioCompositorInit(ThunderAudioCompositor* self, struct ImprintAllocator* memory)
{
    self->output = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderMixSample, THUNDER_ATOM_SAMPLE_COUNT);
    self->output16Bit = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderSample, THUNDER_ATOM_SAMPLE_COUNT);
    self->nodesMaxCount = 10;
    self->nodes = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderAudioNode, self->nodesMaxCount);
    for (size_t i = 0; i < self->nodesMaxCount; ++i) {
        self->nodes[i]._self = 0;
        self->nodes[i].output = 0;
    }

    thunderAudioBufferInit(&self->buffer, memory, THUNDER_ATOM_COUNT_BUFFER, THUNDER_ATOM_SAMPLE_COUNT);

    self->mixDownVolume = 1;
}

void thunderAudioCompositorFree(ThunderAudioCompositor* self)
{
    tc_free(self->output);
    tc_free(self->nodes);
    thunderAudioBufferFree(&self->buffer);
}
