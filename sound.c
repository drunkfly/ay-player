#include "player.h"
#include "ayumi/ayumi.h"
#include "pt3.h"
#include <assert.h>

#define FREQ 44100
#define CLOCKRATE 1750000//2000000

extern PT3 pt3[3];

static SDL_AudioSpec g_audioSpec;
static SDL_AudioDeviceID g_audioDev;

static unsigned char soundBuf[1048576];
static size_t soundBufPos;
static size_t soundBufSize;

static struct ayumi ayumi[3];

static void initAyumi(struct ayumi* ayumi)
{
    int r = ayumi_configure(ayumi, 0, CLOCKRATE, FREQ);
    (void)r;
    assert(r != 0);
    ayumi_set_pan(ayumi, 0, 0.1, 0);
    ayumi_set_pan(ayumi, 1, 0.5, 0);
    ayumi_set_pan(ayumi, 2, 0.9, 0);
}

static double clamp(double x)
{
    if (x < -1.0)
        x = -1.0;
    else if (x > 1.0)
        x = 1.0;
    return x;
}

static void updateAyumi(struct ayumi* ayumi, const PT3* pt3)
{
    ayumi_set_tone(ayumi, 0, pt3->AY.tonA);
    ayumi_set_tone(ayumi, 1, pt3->AY.tonB);
    ayumi_set_tone(ayumi, 2, pt3->AY.tonC);
    ayumi_set_noise(ayumi, pt3->AY.noise);
    ayumi_set_mixer(ayumi, 0, ((pt3->AY.mixer     ) & 1), (pt3->AY.mixer >> 3) & 1, (pt3->AY.amplA >> 4));
    ayumi_set_mixer(ayumi, 1, ((pt3->AY.mixer >> 1) & 1), (pt3->AY.mixer >> 4) & 1, (pt3->AY.amplB >> 4));
    ayumi_set_mixer(ayumi, 2, ((pt3->AY.mixer >> 2) & 1), (pt3->AY.mixer >> 5) & 1, (pt3->AY.amplC >> 4));
    ayumi_set_volume(ayumi, 0, pt3->AY.amplA & 0xf);
    ayumi_set_volume(ayumi, 1, pt3->AY.amplB & 0xf);
    ayumi_set_volume(ayumi, 2, pt3->AY.amplC & 0xf);
    ayumi_set_envelope(ayumi, pt3->AY.env);
    if (pt3->AY.envTp != 0xff)
        ayumi_set_envelope_shape(ayumi, pt3->AY.envTp);
}

static void FillAudio(void* udata, Uint8* stream, int len)
{
    Sint16* p = (Sint16*)stream;
    for (; len > 3; len -= 4) {
        Sint16 lValue = 0, rValue = 0;

        ayumi_process(&ayumi[0]);
        ayumi_remove_dc(&ayumi[0]);
        lValue = (Sint16)(clamp(ayumi->left  * 0.1) * 32767.0);
        rValue = (Sint16)(clamp(ayumi->right * 0.1) * 32767.0);

        *p++ = lValue;
        *p++ = rValue;
    }
}

void InitSound()
{
    SDL_AudioSpec audioReq;
    memset(&audioReq, 0, sizeof(audioReq));
    audioReq.freq = FREQ;
    audioReq.format = AUDIO_S16;
    audioReq.channels = 2;
    audioReq.samples = 241;
    audioReq.callback = FillAudio;

    memset(&g_audioSpec, 0, sizeof(g_audioSpec));
    g_audioDev = SDL_OpenAudioDevice(NULL, 0, &audioReq, &g_audioSpec, 0);

    if (g_audioDev != 0)
        SDL_PauseAudioDevice(g_audioDev, 0);

    initAyumi(&ayumi[0]);
}

void CleanupSound()
{
    if (g_audioDev) {
        SDL_CloseAudioDevice(g_audioDev);
        g_audioDev = 0;
    }
}

void DoRenderSound()
{
    updateAyumi(&ayumi[0], &pt3[0]);
}
