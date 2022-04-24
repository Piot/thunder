#include <stdio.h>
#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

int thunderSoundDriverInit(ThunderSoundDriver* self, thunder_audio_buffer* buffer)
{
    return thunder_pulseaudio_sound_driver_init(&self->pulseaudio_driver, buffer, false);
}

void thunderSoundDriverFree(ThunderSoundDriver* self)
{
    thunder_pulseaudio_sound_driver_free(&self->pulseaudio_driver);
}
