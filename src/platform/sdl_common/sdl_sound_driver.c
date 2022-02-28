#include <thunder/platform/sdl/sound_driver.h>

#include <SDL2/SDL.h>
#include <clog/clog.h>
#include <thunder/sound_buffer.h>
#include <tyran/tyran_types.h>

#include <SDL2/SDL.h>

thunder_sample temp_buf[16 * 1024];

static void fill_f32_buffer_callback(void* _self, Uint8* target, int octet_length)
{
    thunder_sdl_sound_driver* self = _self;

    thunder_audio_buffer* sound_buffer = self->buffer;

    if ((octet_length % 4) != 0) {
        CLOG_ERROR("ERROR!!!!! Wrong octet_length");
    }
    size_t sample_count_to_fill = octet_length / sizeof(float);
    if (sample_count_to_fill > 16 * 1024) {
        CLOG_ERROR("ERROR!!!!! sample_count too big");
        return;
    }
    if ((sample_count_to_fill % 8) != 0) {
        CLOG_ERROR("DIVISON ERR");
        return;
    }

    float* float_target = (float*) target;
    thunder_audio_buffer_read(sound_buffer, temp_buf, sample_count_to_fill);
    size_t spin = sample_count_to_fill / 8;
    thunder_sample* source = temp_buf;
    const float divisor = 33000.0f;
    for (size_t i = 0; i < spin; ++i) {
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
        *float_target++ = *source++ / divisor;
    }
}

static void fill_s16_buffer_callback(void* _self, Uint8* target, int octet_length)
{
    thunder_sdl_sound_driver* self = _self;

    thunder_audio_buffer* sound_buffer = self->buffer;

    if ((octet_length % 4) != 0) {
        CLOG_ERROR("ERROR!!!!! Wrong octet_length");
    }
    size_t sample_count_to_fill = octet_length / sizeof(s16t);
    if (sample_count_to_fill > 16 * 1024) {
        CLOG_ERROR("ERROR!!!!! sample_count too big");
        return;
    }
    if ((sample_count_to_fill % 8) != 0) {
        CLOG_ERROR("DIVISON ERR");
        return;
    }

    thunder_sample_output_s16* int_target = (thunder_sample_output_s16*) target;
    thunder_audio_buffer_read(sound_buffer, int_target, sample_count_to_fill);
}

static void start_playback(thunder_sdl_sound_driver* self)
{
    SDL_PauseAudioDevice(self->device_handle, 0);
}

static void audio_format(SDL_AudioSpec* want, SDL_AudioFormat format, SDL_AudioCallback callback)
{
    SDL_memset(want, 0, sizeof(*want));
    want->freq = 48000;
    want->format = format;
    want->channels = 2;
    want->samples = 8192;
    want->callback = callback;
}

int thunder_sdl_sound_driver_init(thunder_sdl_sound_driver* self, thunder_audio_buffer* buffer,
                                   tyran_boolean use_floats)
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    self->buffer = buffer;
    SDL_AudioFormat want_format = use_floats ? AUDIO_F32 : AUDIO_S16SYS;
    SDL_AudioCallback calback = use_floats ? fill_f32_buffer_callback : fill_s16_buffer_callback;
    SDL_AudioSpec want, have;
    audio_format(&want, want_format, calback);
    want.userdata = self;
    SDL_AudioDeviceID result = SDL_OpenAudioDevice(0, 0, &want, &have, 0);
    if (result <= 0) {
        CLOG_WARN("sdl2: failed to open audio device '%s'", SDL_GetError());
        return -1;
    }
    self->device_handle = result;
    if (have.format != want.format) {
        CLOG_WARN("We didn't get what we wanted.%d", have.format);
        return -2;
    }
    CLOG_VERBOSE("Resulting playback format:%d freq:%d samples:%d", have.format, have.freq, have.samples);
    start_playback(self);

    return 0;
}

void thunder_sdl_sound_driver_free(thunder_sdl_sound_driver* self)
{
    SDL_CloseAudio();
}
