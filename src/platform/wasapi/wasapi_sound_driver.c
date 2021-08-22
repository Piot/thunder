#include <Windows.h>
#include <ole2.h>
#include <audioclient.h>
#include <initguid.h>
#include <mmdeviceapi.h>

#include <stdio.h>
#include <math.h>

static int printfDebug(const char* format, ...)
{
    char str[1024];

    va_list argumentStart;

    va_start(argumentStart, format);

    int returnCode = vsnprintf(str, sizeof(str), format, argumentStart);

    va_end(argumentStart);

    OutputDebugStringA(str);

    return returnCode;
}

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
    if (blockAlign != 4 * 2) {
        printfDebug("error\n");

    }
    printfDebug("filling with %d samples \n", blockAlignFrames);
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
    printfDebug("%s: channels %d freq %d bits %d (%d)\n", prefix, waveFormat->nChannels, waveFormat->nSamplesPerSec,
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
        printfDebug("Supported!\n");
    } else if (hr == S_FALSE) {
        printfDebug("Not supported, but I have a close match: ");
    } else {
        printfDebug("Not supported at all. No close match. \n");
    }

    printFormat(closestMatch, "closestMatch");

    return hr == S_OK;
}

int test()
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
       checkFormat(audioClient, 48000, 32);
       checkFormat(audioClient, 48000, 16);
       checkFormat(audioClient, 41000, 16);

   setupStereoFormat(waveFormat, 44100, 32);
#define MINIMUM_BUFFER_SIZE_IN_MS (80)
   CHECKRETURN(initializeFormat(audioClient, MINIMUM_BUFFER_SIZE_IN_MS, (WAVEFORMATEX*) waveFormat))

   UINT32 maxBufferFrameCount;
   CHECKRETURN(audioClient->lpVtbl->GetBufferSize(audioClient, &maxBufferFrameCount))
   printfDebug("blockAlignFrames: %d\n", maxBufferFrameCount);

    IAudioRenderClient* pRenderClient;
    CHECKRETURN(getRenderClient(audioClient, &pRenderClient))
    printfDebug("\nWorked\n");

    fillBufferWithSine(pRenderClient, maxBufferFrameCount, waveFormat->Format.nBlockAlign);

   CHECKRETURN_HRESULT(audioClient->lpVtbl->Start(audioClient))

       for (int i = 0; i < 1000; ++i) {
           Sleep(1);
           UINT32 paddingFrames;
           audioClient->lpVtbl->GetCurrentPadding(audioClient, &paddingFrames);
           UINT32 framesAvailableInBuffer = maxBufferFrameCount - paddingFrames;
           fillBufferWithSine(pRenderClient, framesAvailableInBuffer, waveFormat->Format.nBlockAlign);
       }

   CHECKRETURN_HRESULT(audioClient->lpVtbl->Stop(audioClient))

       return 0;
}