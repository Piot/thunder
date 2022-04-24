#include <stdio.h>
#include <thunder/platform/xaudio2/xaudio2_sound_driver.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_driver.h>

int thunderSoundDriverInit(ThunderSoundDriver* self, thunder_audio_buffer* buffer)
{
    return thunderXaudio2SoundDriverInit(&self->xaudio2Driver, buffer, false);
}

void thunderSoundDriverFree(ThunderSoundDriver* self)
{
    thunderXaudio2SoundDriverFree(&self->xaudio2Driver);
}
