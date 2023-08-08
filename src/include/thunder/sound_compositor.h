/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_compositor_h
#define thunder_sound_compositor_h

#include <stddef.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_types.h>

struct ImprintMemory;
struct ThunderAudioNode;

typedef struct ThunderAudioCompositor {
    struct ThunderAudioNode* nodes;
    size_t nodes_max_count;
    size_t nodes_count;
    float mix_down_volume;
    thunder_audio_buffer buffer;
    ThunderMixSample* output;
    ThunderSample* output_16_bit;
} ThunderAudioCompositor;

void thunder_audio_compositor_init(ThunderAudioCompositor* self, struct ImprintAllocator* memory);

void thunder_audio_compositor_update(ThunderAudioCompositor* self);

void thunder_audio_compositor_reload(ThunderAudioCompositor* self);

void thunder_audio_compositor_free(ThunderAudioCompositor* self);

#endif
