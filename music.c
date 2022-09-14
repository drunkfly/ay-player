#include "player.h"
#include "pt3.h"

static const unsigned char* currentMusic;
PT3 pt3[3];

void PlayMusic(const unsigned char* data)
{
    if (currentMusic != data) {
        currentMusic = data;
        PT3_init(&pt3[0], data);
    }
}

void StopMusic()
{
    currentMusic = NULL;
}

void DoMusicFrame()
{
    if (currentMusic != NULL)
        PT3_play(&pt3[0]);
    else
        PT3_mute(&pt3[0]);
}
