#include <stdio.h>
#include <thunder/platform/xaudio2/xaudio2_sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

int thunder_sound_driver_init(thunder_sound_driver* self, thunder_audio_buffer* buffer)
{
    return thunder_xaudio2_sound_driver_init(&self->xaudio2_driver, buffer, false);
}

void thunder_sound_driver_free(thunder_sound_driver* self)
{
    thunder_xaudio2_sound_driver_free(&self->xaudio2_driver);
}
