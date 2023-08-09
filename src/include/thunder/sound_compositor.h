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
    size_t nodesMaxCount;
    size_t nodesCount;
    float mixDownVolume;
    ThunderAudioBuffer buffer;
    ThunderMixSample* output;
    ThunderSample* output16Bit;
} ThunderAudioCompositor;

void thunderAudioCompositorInit(ThunderAudioCompositor* self, struct ImprintAllocator* memory);

void thunderAudioCompositorUpdate(ThunderAudioCompositor* self);

void thunderAudioCompositorReload(ThunderAudioCompositor* self);

void thunderAudioCompositorFree(ThunderAudioCompositor* self);

#endif
