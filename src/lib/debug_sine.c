/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <basal/math.h>
#include <stdio.h>
#include <thunder/audio_node.h>
#include <thunder/sound_module.h>
#include <thunder/sound_types.h>
#include <tiny-libc/tiny_libc.h>

#include <math.h>

typedef struct debug_sine {
    uint64_t time;
    uint64_t timeFactor;
    uint64_t amplitude;
    uint64_t amplitudeFactor;
    float offset;
} debug_sine;

static void generate_sine(void* _self, ThunderSample* output, int sample_count)
{
    // printf("sine:%d\n", sample_count);
    debug_sine* self = _self;
    for (int i = 0; i < sample_count; ++i) {
        float angle = self->time % 628318 / 100000.0f;
        float f = sinf(angle);
        float amplitude = (float) self->amplitude / 1000000.0f;

        output[i] = ((ThunderSample) (f * 32767.0f * amplitude));
        self->time += self->timeFactor;
        self->amplitude += self->amplitudeFactor;
        if (self->amplitude > 1000000) {
            self->amplitude = 1000000;
        }
    }
}

static ThunderAudioNode make_sine_node(uint64_t offset, uint64_t factor, uint64_t amplitudeFactor)
{
    ThunderAudioNode self;
    debug_sine* data = tc_malloc_type(debug_sine);

    self.output = generate_sine;
    self._self = data;
    self.is_playing = 1;
    self.channel_count = 1;
    self.pan = 0;
    self.volume = 0.5f;

    data->offset = (float) offset;
    data->timeFactor = factor;
    data->amplitude = 10;
    data->amplitudeFactor = amplitudeFactor;
    data->time = offset;

    return self;
}

static void add_sine(ThunderSoundModule* self, uint64_t offset, uint64_t factor, uint64_t amplitudeFactor)
{
    (void) offset;

    ThunderAudioNode node = make_sine_node(offset, factor, amplitudeFactor);
    thunderSoundModuleAddNode(self, node);
}

void thunderSoundModuleDebugSineWave(ThunderSoundModule* self, struct ImprintMemory* memory)
{
    (void) memory;

    const double base = 4300;
    const uint64_t first = ((uint64_t) (1.4983 * base));
    const uint64_t second = ((uint64_t) (1.2599 * base));
    const uint64_t third = ((uint64_t) (2.0 * base));

    add_sine(self, 0, (uint64_t) base, 30);
    add_sine(self, 1120, first, 20);
    add_sine(self, 530, second, 4);
    add_sine(self, 130, third, 6);
}
