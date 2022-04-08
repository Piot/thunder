#include <stdio.h>
#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

int thunder_sound_driver_init(ThunderSoundDriver* self, thunder_audio_buffer* buffer)
{
    return thunder_pulseaudio_sound_driver_init(&self->pulseaudio_driver, buffer, false);
}

void thunder_sound_driver_free(ThunderSoundDriver* self)
{
    thunder_pulseaudio_sound_driver_free(&self->pulseaudio_driver);
}
