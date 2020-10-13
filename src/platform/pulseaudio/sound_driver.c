#include <thunder/sound_driver.h>
#include <thunder/sound_buffer.h>

void thunder_sound_driver_init(thunder_sound_driver* self, thunder_audio_buffer* buffer)
{
	thunder_pulseaudio_sound_driver_init(&self->pulseaudio_driver, buffer, TYRAN_FALSE);
}

void thunder_sound_driver_update(thunder_sound_driver* self)
{
	thunder_pulseaudio_sound_driver_update(&self->pulseaudio_driver);
}

void thunder_sound_driver_free(thunder_sound_driver* self)
{
	thunder_pulseaudio_sound_driver_free(&self->pulseaudio_driver);
}
