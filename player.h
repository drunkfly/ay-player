
#include <SDL.h>
#include <SDL_main.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   200

void InitSound();
void CleanupSound();
void DoRenderSound();

void PlayMusic(const unsigned char* data);
void StopMusic();
void DoMusicFrame();

void ErrorExit(const char* fmt, ...);
