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
#include <clog/clog.h>
#include <imprint/allocator.h>
#include <thunder/sound_buffer.h>

void thunder_audio_buffer_write(thunder_audio_buffer* self, const ThunderSample* samples, int sample_count)
{
    CLOG_ASSERT(self->atom_size == sample_count, "Wrong store size:%d", sample_count);

    if (self->write_index == self->read_index) {
        // return;
    }

    int index = self->write_index;

    ThunderSampleOutputS16* buffer = self->buffers[index];

    tc_memcpy_octets(buffer, samples, sample_count * sizeof(ThunderSample));

    if (!self->read_buffer) {
        self->read_buffer = self->buffers[0];
        self->read_buffer_samples_left = self->atom_size;
    }

    self->write_index++;
    self->write_index %= self->buffer_count;
}

// NOTE: IMPORTANT: You can not allocate mewmory or use mutex when reading!
void thunder_audio_buffer_read(thunder_audio_buffer* self, ThunderSampleOutputS16* output, int sample_count)
{
    int samples_to_read = sample_count;

    if (samples_to_read > self->read_buffer_samples_left) {
        samples_to_read = self->read_buffer_samples_left;
    }
    tc_memcpy_type(ThunderSampleOutputS16, output, self->read_buffer, samples_to_read);
    sample_count -= samples_to_read;
    self->read_buffer_samples_left -= samples_to_read;

    if (self->read_buffer_samples_left == 0) {
        if (self->write_index == self->read_index) {
            ThunderSampleOutputS16* zero = output + samples_to_read;
            tc_mem_clear_type_n(zero, sample_count);
            return;
        }
        self->read_index++;
        self->read_index %= self->buffer_count;
        self->read_buffer = self->buffers[self->read_index];
        self->read_buffer_samples_left = self->atom_size;
    } else {
        self->read_buffer += samples_to_read;
    }

    if (sample_count > 0) {
        thunder_audio_buffer_read(self, output + samples_to_read, sample_count);
    }
}

void thunder_audio_buffer_init(thunder_audio_buffer* self, struct ImprintAllocator* memory, int bufferCount, int atom_size)
{
    self->buffer_count = bufferCount;
    self->atom_size = atom_size;
    self->read_index = -1;
    self->write_index = 0;
    self->buffers = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderSampleOutputS16*, self->buffer_count);
    for (int i = 0; i < self->buffer_count; ++i) {
        self->buffers[i] = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderSampleOutputS16, atom_size);
    }
    self->read_buffer = 0;
    self->read_buffer_samples_left = 0;
}

float thunder_audio_buffer_percentage_full(thunder_audio_buffer* self)
{
    int diff = thunder_audio_buffer_atoms_full(self);
    return diff / (float) self->buffer_count;
}

int thunder_audio_buffer_atoms_full(thunder_audio_buffer* self)
{
    int diff;

    if (self->write_index == self->read_index) {
        return 0;
    }

    if (self->write_index > self->read_index) {
        diff = self->write_index - self->read_index;
    } else {
        diff = self->buffer_count - self->read_index + self->write_index;
    }

    return diff;
}

void thunder_audio_buffer_free(thunder_audio_buffer* self)
{
    for (int i = 0; i < self->buffer_count; ++i) {
        tc_free(self->buffers[i]);
    }
    tc_free(self->buffers);
    self->buffer_count = 0;
}
