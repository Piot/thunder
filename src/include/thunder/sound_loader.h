/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_loader_h
#define thunder_sound_loader_h

#include <stdbool.h>

struct thunder_sound_wave;
struct ImprintMemory;

void thunder_sound_loader_load(struct ImprintMemory* memory, const uint8_t* data, int octets,
                               struct thunder_sound_wave* sound);

#endif
