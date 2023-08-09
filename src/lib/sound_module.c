/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <thunder/audio_node.h>
#include <thunder/sound_compositor.h>
#include <thunder/sound_module.h>

void thunderSoundModuleUpdate(ThunderSoundModule* _self)
{
    ThunderSoundModule* self = _self;

    size_t atomsFull = thunderAudioBufferAtomsFull(&self->compositor.buffer);
    if (atomsFull >= 2) {
        return;
    }

    thunderAudioCompositorUpdate(&self->compositor);
    if (atomsFull < 1) {
        thunderAudioCompositorUpdate(&self->compositor);
    }
}

void thunderSoundModuleAddNode(ThunderSoundModule* self, ThunderAudioNode node)
{
    self->compositor.nodes[self->compositor.nodesCount++] = node;
}

void thunderSoundModuleReload(ThunderSoundModule* self)
{
    thunderAudioCompositorReload(&self->compositor);
}

int thunderSoundModuleInit(ThunderSoundModule* self, struct ImprintAllocator* memory)
{
    thunderAudioCompositorInit(&self->compositor, memory);

    thunderAudioCompositorUpdate(&self->compositor);
    thunderAudioCompositorUpdate(&self->compositor);
    thunderAudioCompositorUpdate(&self->compositor);

    self->initialized = true;

    return 0;
}

int thunderSoundModuleDestroy(ThunderSoundModule* self)
{
    (void) self;
    return 0;
}
