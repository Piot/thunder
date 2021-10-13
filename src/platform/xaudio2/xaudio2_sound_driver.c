#include <WinSock2.h>
#include <Windows.h>
#include <thunder/platform/xaudio2/xaudio2_sound_driver.h>
#include <xaudio2.h>

#include <clog/clog.h>
#include <math.h>
#include <thunder/sound_buffer.h>


typedef struct thunder_xaudio2_callback {
    IXAudio2VoiceCallback IXAudio2VoiceCallback_iface;
    thunder_xaudio2_sound_driver* self;
} thunder_xaudio2_callback;

#define CHECKFAILED(hr)                                                                                                \
    if (FAILED((hr))) {                                                                                                \
        return -1;                                                                                                     \
    }

#define CHECKFAILEDLAST(hr)                                                                                            \
    CHECKFAILED(hr)                                                                                                    \
    return 0;

#define CHECKRETURN(code)                                                                                              \
    if ((errorCode = code) != 0) {                                                                                     \
        return errorCode;                                                                                              \
    }

#define CHECKRETURN_HRESULT(code)                                                                                      \
    {                                                                                                                  \
        HRESULT hr;                                                                                                    \
        if ((hr = code) != S_OK) {   \
            CLOG_WARN("operation failed %08X", hr); \
            return (int) hr;                                                                                           \
        }                                                                                                              \
    }

#define CHECKRETURN_HRESULT_PANIC(code)                                                                                      \
    {                                                                                                                  \
        HRESULT hr;                                                                                                    \
        if ((hr = code) != S_OK) {                                                                                     \
            CLOG_ERROR("operation failed %08X", hr);                                                                    \
        }                                                                                                              \
    }

float g_sineTime = 0;

/*
static int fillBufferWithSine(IAudioRenderClient* renderClient, UINT32 blockAlignFrames, WORD blockAlign)
{
    BYTE* pData;

    CLOG_DEBUG("filling with %d samples \n", blockAlignFrames);
    CHECKRETURN_HRESULT(renderClient->lpVtbl->GetBuffer(renderClient, blockAlignFrames, &pData))

    float* p = (float*) pData;
    float volume = 0.4f;
    for (UINT32 i = 0; i < blockAlignFrames; ++i) {
        float l = sinf(g_sineTime) * volume;
        float r = cosf(g_sineTime / 2.0f) * volume;
        *p++ = l;
        *p++ = r;
        g_sineTime += 6.28f / 100.0f;
    }

    DWORD flags = 0; // AUDCLNT_BUFFERFLAGS_SILENT
    renderClient->lpVtbl->ReleaseBuffer(renderClient, blockAlignFrames, flags);

    return 0;
}

*/
static void printFormat(const WAVEFORMATEXTENSIBLE* waveFormatExtensible, const char* prefix)
{
    const WAVEFORMATEX* waveFormat = &waveFormatExtensible->Format;
    CLOG_DEBUG("%s: channels %d freq %d bits %d (%d)\n", prefix, waveFormat->nChannels, waveFormat->nSamplesPerSec,
               waveFormat->wBitsPerSample, waveFormat->wFormatTag);
}

static void setupStereoFormatBase(WAVEFORMATEX* proposedFormat, UINT32 frequency, WORD bits)
{
    WORD format = WAVE_FORMAT_PCM;
    if (bits == 32) {
        format = WAVE_FORMAT_IEEE_FLOAT;
    }

    proposedFormat->wFormatTag = format;
    proposedFormat->nChannels = 2;
    proposedFormat->nSamplesPerSec = frequency;
    proposedFormat->wBitsPerSample = bits;
    proposedFormat->nBlockAlign = proposedFormat->nChannels * (proposedFormat->wBitsPerSample / 8);
    proposedFormat->nAvgBytesPerSec = proposedFormat->nSamplesPerSec * proposedFormat->nBlockAlign;
    proposedFormat->cbSize = 0;
    //sizeof(WAVEFORMATEX);
}

static void setupStereoFormat(WAVEFORMATEXTENSIBLE* proposedFormat, UINT32 frequency, WORD bits)
{

    setupStereoFormatBase(&proposedFormat->Format, frequency, bits);
    proposedFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    proposedFormat->Samples.wReserved = 0;
    proposedFormat->Samples.wSamplesPerBlock = 2;
    proposedFormat->Samples.wValidBitsPerSample = bits;

    proposedFormat->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
}

static void WINAPI onVoiceProcessingPassStart(IXAudio2VoiceCallback* iface, UINT32 BytesRequired)
{
    thunder_xaudio2_sound_driver* self = ((thunder_xaudio2_callback*) iface)->self;

}

static void WINAPI onVoiceProcessingPassEnd(IXAudio2VoiceCallback* iface)
{
    thunder_xaudio2_sound_driver* self = ((thunder_xaudio2_callback*) iface)->self;

}

static void WINAPI onStreamEnd(IXAudio2VoiceCallback* iface)
{
    CLOG_DEBUG("xaudio: streamEnd")
}

    XAUDIO2_BUFFER g_xAudioBuffer;

static void WINAPI onBufferStart(IXAudio2VoiceCallback* iface, void* pBufferContext)
{
}

static void WINAPI onBufferEnd(IXAudio2VoiceCallback* iface, void* pBufferContext)
{
    thunder_xaudio2_sound_driver* self = ((thunder_xaudio2_callback*) iface)->self;

    XAUDIO2_BUFFER xAudioBuffer2 = {0};
    BYTE* buffer = self->buffers[self->activeBuffer];
    xAudioBuffer2.AudioBytes = self->bufferAtomSizeInOctets;
    xAudioBuffer2.pAudioData = buffer;
    thunder_audio_buffer_read(self->buffer, buffer, self->bufferAtomSizeInOctets/2);
    CHECKRETURN_HRESULT_PANIC(self->sourceVoice->lpVtbl->SubmitSourceBuffer(self->sourceVoice, &xAudioBuffer2, 0))
    self->activeBuffer = !self->activeBuffer;
}

static void WINAPI onLoopEnd(IXAudio2VoiceCallback* This, void* pBufferContext)
{
    CLOG_DEBUG("xaudio: loopEnd")
}

static void WINAPI onVoiceError(IXAudio2VoiceCallback* This, void* pBuffercontext, HRESULT Error)
{
    CLOG_DEBUG("xaudio: voiceError")
}



static IXAudio2VoiceCallbackVtbl vcb_vtbl = {onVoiceProcessingPassStart,
                                             onVoiceProcessingPassEnd,
                                             onStreamEnd,
                                             onBufferStart,
                                             onBufferEnd,
                                             onLoopEnd,
                                             onVoiceError};


static struct thunder_xaudio2_callback callbacks = {&vcb_vtbl};


static int init(thunder_xaudio2_sound_driver* self)
{
    //CHECKRETURN_HRESULT(CoInitializeEx(0, COINIT_MULTITHREADED))
    CHECKRETURN_HRESULT(CoInitialize(0))

    CLOG_DEBUG("xaudio: create interface")
    CHECKRETURN_HRESULT(XAudio2Create(&self->xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))

    WAVEFORMATEX sourceFormat;
    setupStereoFormatBase(&sourceFormat, 48000, 16);

    self->bufferAtomSizeInOctets = 800 * 4;
    self->bufferAtomSizeInBlocks = self->bufferAtomSizeInOctets / sourceFormat.nBlockAlign;
    self->buffers[0] = calloc(1, self->bufferAtomSizeInOctets);
    self->buffers[1] = calloc(1, self->bufferAtomSizeInOctets);
    self->activeBuffer = 0;

    CLOG_DEBUG("xaudio: create default mastering voice")
    CHECKRETURN_HRESULT(self->xaudio2->lpVtbl->CreateMasteringVoice(
        self->xaudio2, &self->masteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, 0, 0))
    
    callbacks.self = self;
    CLOG_DEBUG("xaudio: create voice")
    CHECKRETURN_HRESULT(self->xaudio2->lpVtbl->CreateSourceVoice(self->xaudio2, &self->sourceVoice, &sourceFormat, 0,
                                                                 1.0, &callbacks, 0, 0))

    CLOG_DEBUG("xaudio: start engine")
    CHECKRETURN_HRESULT(IXAudio2_StartEngine(self->xaudio2))

    XAUDIO2_BUFFER xAudioBuffer;
    tc_mem_clear_type(&xAudioBuffer);
    xAudioBuffer.AudioBytes = self->bufferAtomSizeInOctets;
    xAudioBuffer.pAudioData = self->buffers[self->activeBuffer];


    CHECKRETURN_HRESULT(self->sourceVoice->lpVtbl->SubmitSourceBuffer(self->sourceVoice, &xAudioBuffer, 0))

    self->activeBuffer = !self->activeBuffer;
    XAUDIO2_BUFFER xAudioBuffer2;
    tc_mem_clear_type(&xAudioBuffer2);
    xAudioBuffer2.AudioBytes = self->bufferAtomSizeInOctets;
    xAudioBuffer2.pAudioData = self->buffers[self->activeBuffer];
    self->activeBuffer = !self->activeBuffer;

    CHECKRETURN_HRESULT(self->sourceVoice->lpVtbl->SubmitSourceBuffer(self->sourceVoice, &xAudioBuffer2, 0))
    CLOG_DEBUG("xaudio: start source voice")
    CHECKRETURN_HRESULT(IXAudio2SourceVoice_Start(self->sourceVoice, 0, XAUDIO2_COMMIT_NOW))
    CLOG_DEBUG("xaudio: initialization worked")

    return 0;
}

uint32_t g_resampleCounter = 0;

static int callback(thunder_xaudio2_sound_driver* self)
{
    /*
    thunder_sample_output_s16* source = tempBuffer;
    for (size_t i = 0; i < framesAvailableInBuffer; ++i) {
        float v = *source++ / 32768.0f;
        float r = *source++ / 32768.0f;
        g_resampleCounter++;
        if ((g_resampleCounter % 12) == 0) {
            v = (v + (*source++ / 32768.0f)) / 2.0f;
            r = (r + (*source++ / 32768.0f)) / 2.0f;
        }
        *target++ = v;
        *target++ = r;
    }
    */
}

int thunder_xaudio2_sound_driver_init(thunder_xaudio2_sound_driver* self, struct thunder_audio_buffer* buffer,
                                      tyran_boolean use_floats)
{
    CLOG_DEBUG("xaudio2 init");
    self->buffer = buffer;
    return init(self);
}

void thunder_xaudio2_sound_driver_free(thunder_xaudio2_sound_driver* self)
{
    IXAudio2_StopEngine(self->xaudio2);
    self->xaudio2 = 0;
    self->sourceVoice = 0;
}
