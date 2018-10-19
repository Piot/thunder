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
#include <burst/burst_file_loader.h>
#include <thunder/sound_loader.h>
#include <thunder/sound_module.h>
#include <tyran/tyran_log.h>
#include <tyran/tyran_memory.h>

tyran_memory global_mem;
thunder_sound_module module;
thunder_sound_module_wave true_wave;
burst_file_loader loader;

void ogg_file_loaded(void* user_data, const uint8_t* octets, int32_t result)
{
	thunder_sound_module_wave* second_wave = &module.waves[1];
	thunder_sound_wave* wave = &second_wave->wave;

	thunder_sound_loader_load(&global_mem, octets, result, wave);
	thunder_sound_module_play_sound(&module, 1, 1, 1.0f, 100000, TYRAN_TRUE);
}

void load_sound()
{
	burst_file_loader_init(&loader);
	burst_file_loader_load(&loader, "test.ogg", 0, ogg_file_loaded);
}

void g_breath_init(int width, int height)
{
	tyran_memory_init(&global_mem, 64 * 1024 * 1024, "just an app");
	thunder_sound_module_init(&module, &global_mem);
	thunder_sound_module_wave* module_wave = &module.waves[0];

#define MAX_COUNT (88)
	thunder_sample test[MAX_COUNT];
	for (int i = 0; i < MAX_COUNT; ++i) {
		tyran_number f = sin((6.28f * (tyran_number) i) / (tyran_number) MAX_COUNT);
		test[i] = f * 32767;
	}
	thunder_sound_wave_init(&module_wave->wave, &global_mem, test, MAX_COUNT, 1);
	// thunder_sound_module_play_sound(&module, 0, 0, 1.0f, 100000, TYRAN_TRUE);
	load_sound();
}

void g_breath_draw()
{
	thunder_sound_module_update(&module);

	burst_file_loader_poll(&loader);
}
