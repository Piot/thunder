/*

MIT License

Copyright (c) 2012 Peter Bjorklund

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef thunder_sound_module_h
#define thunder_sound_module_h

#include <thunder/sound_driver.h>

#include <thunder/sound_compositor.h>

struct ImprintMemory;
struct thunder_audio_node_player;

typedef struct thunder_sound_module_sound {
    bool is_used;
    bool should_play;
    bool waiting_for_play;
    float time;
    float volume;
    float pan;
    int loop;
    struct thunder_sound_module_sound* sync_with_sound;
    struct thunder_audio_compositor_node* node;
} thunder_sound_module_sound;

typedef struct thunder_sound_module {
    thunder_sound_driver driver;
    struct ImprintMemory* memory;
    thunder_audio_compositor compositor;
    bool initialized;
} thunder_sound_module;

int thunder_sound_module_init(thunder_sound_module* _self, struct ImprintMemory* memory);

void thunder_sound_module_update(thunder_sound_module* _self);

void thunder_sound_module_play_sound(thunder_sound_module* self, int sound_id, int wave_id, float volume,
                                     int loop, bool should_play);

void thunder_sound_module_debug_sine_wave(thunder_sound_module* self, struct ImprintMemory* memory);

void thunder_sound_module_add_node(thunder_sound_module* self, struct thunder_audio_node node);
void thunder_sound_module_reload(thunder_sound_module* self);

#endif
