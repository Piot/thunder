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

    size_t atomsFull = thunder_audio_buffer_atoms_full(&self->compositor.buffer);
    if (atomsFull >= 2) {
        return;
    }

    thunder_audio_compositor_update(&self->compositor);
    if (atomsFull < 1) {
        thunder_audio_compositor_update(&self->compositor);
    }
}

void thunderSoundModuleAddNode(ThunderSoundModule* self, ThunderAudioNode node)
{
    self->compositor.nodes[self->compositor.nodes_count++] = node;
}

void thunderSoundModuleReload(ThunderSoundModule* self)
{
    thunder_audio_compositor_reload(&self->compositor);
}

int thunderSoundModuleInit(ThunderSoundModule* self, struct ImprintAllocator* memory)
{
    thunder_audio_compositor_init(&self->compositor, memory);

    thunder_audio_compositor_update(&self->compositor);
    thunder_audio_compositor_update(&self->compositor);
    thunder_audio_compositor_update(&self->compositor);

    self->initialized = true;

    return 0;
}

int thunderSoundModuleDestroy(ThunderSoundModule* self)
{
    (void) self;
    return 0;
}
