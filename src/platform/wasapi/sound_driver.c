#include <stdio.h>
#include <thunder/platform/wasapi/wasapi_sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

int thunder_sound_driver_init(thunder_sound_driver* self, thunder_audio_buffer* buffer)
{
    return thunder_wasapi_sound_driver_init(&self->wasapi_driver, buffer, TYRAN_FALSE);
}

void thunder_sound_driver_free(thunder_sound_driver* self)
{
    thunder_wasapi_sound_driver_free(&self->wasapi_driver);
}
