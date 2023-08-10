/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_audio_node_h
#define thunder_audio_node_h

#include <thunder-audio-buffer/sound_types.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*ThunderAudioNodeOutputFunc)(void* self, ThunderSample* sample, size_t sample_count);

typedef struct ThunderAudioNode {
    ThunderAudioNodeOutputFunc output;
    void* _self;
    bool is_playing;
    int channel_count;
    int pan;
    float volume;
} ThunderAudioNode;

void thunderAudioNodeInit(ThunderAudioNode* self, ThunderAudioNodeOutputFunc func, void* other_self);

#endif
