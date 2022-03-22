#include <thunder/platform/wasapi/wasapi_sound_driver.h>
#include <WinSock2.h>
#include <Windows.h>
#include <ole2.h>
#include <initguid.h>
#include <stdio.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

#include <thunder/sound_buffer.h>
#include <math.h>
#include <clog/clog.h>


static int createEnumerator(IMMDeviceEnumerator** target)
{
    const CLSID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
    const IID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};

    HRESULT hres = CoCreateInstance(
           &CLSID_MMDeviceEnumerator, NULL,
           CLSCTX_ALL, &IID_IMMDeviceEnumerator,
                                    (void**) target);
     if (FAILED(hres)) {
         return -1;
     }

     return 0;
}

#define CHECKFAILED(hr)                                                                                                \
    if (FAILED((hr))) {                                                                                                \
        return -1; \
    }
  
#define CHECKFAILEDLAST(hr)                                                                                                \
   CHECKFAILED(hr)                                                                                             \
    return 0;                                                                                                     \

static int getDefaultEndpoint(IMMDeviceEnumerator* enumerator, IMMDevice** outDevice)
{
    HRESULT hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, outDevice);
    CHECKFAILEDLAST(hr)
}

static int activateDevice(IMMDevice* device, IAudioClient** outAudioClient)
{
    const IID IID_IAudioClient = {0x1cb9ad4c, 0xdbfa, 0x4c32, 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2};
    HRESULT hr = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**) outAudioClient);
    CHECKFAILEDLAST(hr)
}

static int getMixFormat(IAudioClient* audioClient, WAVEFORMATEX** pwfx)
{
   HRESULT hr = audioClient->lpVtbl->GetMixFormat(audioClient, pwfx);
   CHECKFAILEDLAST(hr)
}

static int initializeFormat(IAudioClient* audioClient, WORD minimumBufferSizeInMs, WAVEFORMATEX* pwfx)
{
    // Windows Reference Time is in 100 nanosecond units
#define REFERENCE_TIME_UNITS_PER_MILLISECOND (10000)

     HRESULT hr = audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, 0,
                                                 REFERENCE_TIME_UNITS_PER_MILLISECOND * minimumBufferSizeInMs, 0, pwfx,
                                                 0);
    CHECKFAILEDLAST(hr)
}

static int getRenderClient(IAudioClient* audioClient, IAudioRenderClient** pRenderClient)
{
    const IID IID_IAudioRenderClient = {0xf294acfc, 0x3146, 0x4483, {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}};

    HRESULT hr = audioClient->lpVtbl->GetService(audioClient, &IID_IAudioRenderClient, (void**) pRenderClient);
    CHECKFAILEDLAST(hr)
}

#define CHECKRETURN(code)                                                                                              \
    if ((errorCode = code) != 0) {                                                                                     \
        return errorCode; \
    }

#define CHECKRETURN_HRESULT(code)                                                                                              \
    { \
        HRESULT hr; \
        if ((hr = code) != S_OK) {                                                                                     \
            return (int) hr; \
        }\
    }

float g_sineTime = 0;

static int fillBufferWithSine(IAudioRenderClient* renderClient, UINT32 blockAlignFrames, WORD blockAlign)
{
    BYTE* pData;

    CLOG_DEBUG("filling with %d samples \n", blockAlignFrames);
    CHECKRETURN_HRESULT(renderClient->lpVtbl->GetBuffer(renderClient, blockAlignFrames, &pData))

    float* p = (float*) pData;
    float volume = 0.4f;
    for (UINT32 i = 0; i < blockAlignFrames; ++i) {
        float l = sinf(g_sineTime) * volume;
        float r = cosf(g_sineTime/2.0f) * volume;
        *p++ = l;
        *p++ = r;
        g_sineTime += 6.28f / 100.0f;
    }


    DWORD flags = 0; // AUDCLNT_BUFFERFLAGS_SILENT
    renderClient->lpVtbl->ReleaseBuffer(renderClient, blockAlignFrames, flags);

    return 0;
}

static void printFormat(const WAVEFORMATEXTENSIBLE* waveFormatExtensible, const char* prefix)
{
    const WAVEFORMATEX* waveFormat = &waveFormatExtensible->Format;
    CLOG_DEBUG("%s: channels %d freq %d bits %d (%d)\n", prefix, waveFormat->nChannels, waveFormat->nSamplesPerSec,
                waveFormat->wBitsPerSample, waveFormat->wFormatTag);
}

static void setupStereoFormat(WAVEFORMATEXTENSIBLE* proposedFormat, UINT32 frequency, WORD bits)
{
    WORD format = WAVE_FORMAT_PCM;

    if (bits == 32) {
        format = WAVE_FORMAT_IEEE_FLOAT;
    }

    proposedFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    proposedFormat->Samples.wReserved = 0;
    proposedFormat->Samples.wSamplesPerBlock = 2;
    proposedFormat->Samples.wValidBitsPerSample = bits;
    proposedFormat->Format.nChannels = 2;
    proposedFormat->Format.nSamplesPerSec = frequency;
    proposedFormat->Format.wBitsPerSample = bits;
    proposedFormat->Format.nAvgBytesPerSec = proposedFormat->Format.wBitsPerSample / 8 *
                                              proposedFormat->Format.nChannels *
                                              proposedFormat->Format.nSamplesPerSec;

    proposedFormat->Format.nBlockAlign = proposedFormat->Format.nChannels * proposedFormat->Format.wBitsPerSample /
                                          8;
    proposedFormat->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
}

static int checkFormat(IAudioClient* audioClient, UINT32 frequency, WORD bits)
{
    AUDCLNT_SHAREMODE shareMode = AUDCLNT_SHAREMODE_SHARED;
    WAVEFORMATEXTENSIBLE proposedFormat;
    WAVEFORMATEXTENSIBLE* closestMatch;

    setupStereoFormat(&proposedFormat, frequency, bits);

    printFormat(&proposedFormat, "proposedFormat");       
    HRESULT hr = audioClient->lpVtbl->IsFormatSupported(audioClient, shareMode, (WAVEFORMATEX*) &proposedFormat,
                                                        (WAVEFORMATEX**) &closestMatch);
    if (hr == S_OK) {
        CLOG_DEBUG("Supported!\n");
    } else if (hr == S_FALSE) {
        CLOG_DEBUG("Not supported, but I have a close match: ");
    } else {
        CLOG_DEBUG("Not supported at all. No close match. \n");
    }

    printFormat(closestMatch, "closestMatch");

    return hr == S_OK;
}

static int init(thunder_wasapi_sound_driver* self)
{
    CHECKRETURN_HRESULT(CoInitialize(0))

    IMMDeviceEnumerator* enumerator;
    int errorCode;
    if ((errorCode = createEnumerator(&enumerator)) != 0)
    {
        return errorCode;
    }

     IMMDevice* device;

    if ((errorCode = getDefaultEndpoint(enumerator, &device)) != 0) {
        return errorCode;
    }

     IAudioClient* audioClient;
        if ((errorCode = activateDevice(device, &audioClient)) != 0) {
        return errorCode;
    }

        WAVEFORMATEXTENSIBLE* waveFormat;
    if ((errorCode = getMixFormat(audioClient, (WAVEFORMATEX**) &waveFormat)) != 0) {
            return errorCode;
        }


        printFormat(waveFormat, "getMixFormat");

   //setupStereoFormat(waveFormat, 44100, 32);
#define MINIMUM_BUFFER_SIZE_IN_MS (80)
   CHECKRETURN(initializeFormat(audioClient, MINIMUM_BUFFER_SIZE_IN_MS, (WAVEFORMATEX*) waveFormat))

   UINT32 maxBufferFrameCount;
   CHECKRETURN(audioClient->lpVtbl->GetBufferSize(audioClient, &maxBufferFrameCount))
   CLOG_DEBUG("blockAlignFrames: %d", maxBufferFrameCount);

    IAudioRenderClient* pRenderClient;
    CHECKRETURN(getRenderClient(audioClient, &pRenderClient))
    CLOG_DEBUG("Worked");

    fillBufferWithSine(pRenderClient, maxBufferFrameCount, waveFormat->Format.nBlockAlign);

    self->audioClient = audioClient;
    self->renderClient = pRenderClient;
    self->device = device;
    self->maxBufferFrameCount = maxBufferFrameCount;

       return 0;
}


    uint32_t g_resampleCounter = 0;

 static int callback(thunder_wasapi_sound_driver* self)
{
     thunder_sample_output_s16 tempBuffer[16 * 1024];
    UINT32 paddingFrames;
    self->audioClient->lpVtbl->GetCurrentPadding(self->audioClient, &paddingFrames);
    UINT32 framesAvailableInBuffer = self->maxBufferFrameCount - paddingFrames;

    thunder_audio_buffer_read(self->buffer, tempBuffer, framesAvailableInBuffer*2);

    BYTE* pData;
    CLOG_DEBUG("filling with %d samples", framesAvailableInBuffer);
    CHECKRETURN_HRESULT(self->renderClient->lpVtbl->GetBuffer(self->renderClient, framesAvailableInBuffer, &pData))


    float* target = pData;
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


    DWORD flags = 0; // AUDCLNT_BUFFERFLAGS_SILENT
    self->renderClient->lpVtbl->ReleaseBuffer(self->renderClient, framesAvailableInBuffer, flags);
}
    

static DWORD __stdcall wasapiThread(LPVOID context)
{
    while (1) {
        Sleep(1);
        callback((thunder_wasapi_sound_driver*) context);
    }
}


int thunder_wasapi_sound_driver_init(thunder_wasapi_sound_driver* self, struct thunder_audio_buffer* buffer,
                                          bool use_floats)
                                          {
    int errorCode;
    CHECKRETURN(init(self));
    self->buffer = buffer;
   CHECKRETURN_HRESULT(self->audioClient->lpVtbl->Start(self->audioClient))
    self->thread = CreateThread(NULL, 0, wasapiThread, self, 0, NULL);

                                          }




void thunder_wasapi_sound_driver_free(thunder_wasapi_sound_driver* self)
{
    CHECKRETURN_HRESULT(self->audioClient->lpVtbl->Stop(self->audioClient))
    CloseHandle(self->thread);
}
