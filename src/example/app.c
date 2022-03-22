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
#include <clog/clog.h>
#include <imprint/memory.h>
#include <thunder/sound_loader.h>
#include <thunder/sound_module.h>

typedef struct example_app {
    thunder_sound_module module;
    ImprintMemory global_mem;
} example_app;

static example_app __app;

void* g_breathe_init(int width, int height)
{
    example_app* self = &__app;

    imprintMemoryInit(&self->global_mem, 64 * 1024 * 1024, "just an self");
    thunder_sound_module_init(&self->module, &self->global_mem);

    thunder_sound_module_debug_sine_wave(&self->module, &self->global_mem);

    return self;
}

int g_breathe_draw(void* _app)
{
    example_app* self = _app;
    thunder_sound_module_update(&self->module);
    return 0;
}

void g_breathe_close(void* _app)
{
}
