#include "player.h"
#include <time.h>

enum {
    DesiredFPS = 50,
    FrameLength = 1000 / DesiredFPS,
};

SDL_Window* g_window;
SDL_Renderer* g_renderer;
SDL_PixelFormat* g_pixelFormat;
SDL_Texture* g_screenTexture;

Uint32 g_prevTicks;
Uint32 g_screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

static void Cleanup(void);

static void Init()
{
    int r = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (r != 0)
        ErrorExit("Unable to initialize SDL: %s", SDL_GetError());

    atexit(Cleanup);

    SDL_ClearError();
    g_window = SDL_CreateWindow("PT3 Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!g_window)
        ErrorExit("Unable to initialize video mode: %s", SDL_GetError());

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer)
        ErrorExit("Unable to initialize renderer: %s", SDL_GetError());

    const unsigned format = SDL_PIXELFORMAT_BGRA32;
    g_pixelFormat = SDL_AllocFormat(format);
    if (!g_pixelFormat)
        ErrorExit("Unable to allocate pixel format: %s", SDL_GetError());

    g_screenTexture = SDL_CreateTexture(g_renderer, format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!g_screenTexture)
        ErrorExit("Unable to create texture: %s", SDL_GetError());

    InitSound();
}

static void Cleanup()
{
    CleanupSound();

    if (g_screenTexture) {
        SDL_DestroyTexture(g_screenTexture);
        g_screenTexture = NULL;
    }

    if (g_pixelFormat) {
        SDL_FreeFormat(g_pixelFormat);
        g_pixelFormat = NULL;
    }

    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }

    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    SDL_Quit();
}

void ErrorExit(const char* fmt, ...)
{
    char buf[2048];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Cleanup();

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", buf);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buf, NULL);

    exit(1);
}

void DoRenderScreen()
{
    /* FIXME */

    SDL_UpdateTexture(g_screenTexture, NULL, g_screenBuffer, SCREEN_WIDTH * sizeof(Uint32));

    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);
    SDL_RenderClear(g_renderer);

    int screenW = 0, screenH = 0;
    SDL_GetRendererOutputSize(g_renderer, &screenW, &screenH);

    SDL_Rect dstRect;
    dstRect.w = SCREEN_WIDTH;
    dstRect.h = SCREEN_HEIGHT;
    dstRect.x = (screenW - dstRect.w) / 2;
    dstRect.y = (screenH - dstRect.h) / 2;
    SDL_RenderCopy(g_renderer, g_screenTexture, NULL, &dstRect);

    SDL_RenderPresent(g_renderer);
}

static int PollEvent()
{
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
        }
    }

    Uint32 ticks = SDL_GetTicks();

    if ((Sint32)(ticks - g_prevTicks) > 100)
        g_prevTicks = ticks;

    if (SDL_TICKS_PASSED(ticks, g_prevTicks + FrameLength)) {
        g_prevTicks += FrameLength;
        return 1;
    }

    return 0;
}

void HandleEvents()
{
    DoMusicFrame();
    DoRenderSound();
    DoRenderScreen();

    while (!PollEvent())
        ;
}

int main(int argc, char** argv)
{
    int i;

    Init();
    g_prevTicks = SDL_GetTicks();

    static unsigned char buf[1048576];
    FILE* f = fopen("music.pt3", "rb");
    fread(buf, 1, sizeof(buf), f);
    fclose(f);
    PlayMusic(buf);

    for (;;)
        HandleEvents();

    return 0;
}
