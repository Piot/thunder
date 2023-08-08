/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_module_h
#define thunder_sound_module_h

#include <thunder/sound_compositor.h>

struct ImprintMemory;
struct thunder_audio_node_player;

typedef struct ThunderSoundModuleSound {
    bool is_used;
    bool should_play;
    bool waiting_for_play;
    float time;
    float volume;
    float pan;
    int loop;
    struct ThunderSoundModuleSound* sync_with_sound;
    struct ThunderAudioCompositorNode* node;
} ThunderSoundModuleSound;

typedef struct ThunderSoundModule {
    ThunderAudioCompositor compositor;
    bool initialized;
} ThunderSoundModule;

int thunderSoundModuleInit(ThunderSoundModule* _self, struct ImprintAllocator* memory);
int thunderSoundModuleDestroy(ThunderSoundModule* _self);
void thunderSoundModuleUpdate(ThunderSoundModule* _self);

void thunderSoundModulePlaySound(ThunderSoundModule* self, int sound_id, int wave_id, float volume, int loop,
                                 bool should_play);

void thunderSoundModuleDebugSineWave(ThunderSoundModule* self, struct ImprintMemory* memory);

void thunderSoundModuleAddNode(ThunderSoundModule* self, struct ThunderAudioNode node);
void thunderSoundModuleReload(ThunderSoundModule* self);

#endif
