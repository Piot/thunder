/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_buffer_h
#define thunder_sound_buffer_h

#include "sound_types.h"

struct ImprintAllocator;

typedef struct thunder_audio_buffer {
    size_t atom_size;
    size_t buffer_count;
    ThunderSampleOutputS16** buffers;
    ThunderSampleOutputS16* read_buffer;
    size_t read_buffer_samples_left;
    size_t write_index;
    size_t read_index;
} thunder_audio_buffer;

void thunder_audio_buffer_write(thunder_audio_buffer* self, const ThunderSample* samples, size_t sample_count);

void thunder_audio_buffer_read(thunder_audio_buffer* self, ThunderSampleOutputS16* output, size_t sample_count);

void thunder_audio_buffer_init(thunder_audio_buffer* self, struct ImprintAllocator* memory, size_t atom_count,
                               size_t atom_size);

void thunder_audio_buffer_free(thunder_audio_buffer* self);

float thunder_audio_buffer_percentage_full(thunder_audio_buffer* self);
size_t thunder_audio_buffer_atoms_full(thunder_audio_buffer* self);

#endif
