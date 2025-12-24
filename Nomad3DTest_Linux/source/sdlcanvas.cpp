

#include "sdlcanvas.h"

void CSDLCanvas ::Init()
{
    if (!m_pCanvas)
    {
        SDL_Init(SDL_INIT_VIDEO);

        m_pSDLWindow = SDL_CreateWindow("Nomad3D Demo",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT, 0);
        m_pSDLRenderer = SDL_CreateRenderer(m_pSDLWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#if defined(NM3D_PIXELFORMAT_565)
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_RGB565,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
        m_pCanvas = (uint16_t *)malloc(NM3D_WINDOW_WIDTH * NM3D_WINDOW_HEIGHT * 2);
#elif defined(NM3D_PIXELFORMAT_555)
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ARGB1555,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
        m_pCanvas = (uint16_t *)malloc(NM3D_WINDOW_WIDTH * NM3D_WINDOW_HEIGHT * 2);
#elif defined(NM3D_PIXELFORMAT888)
#if NM3D_PIXELFORMAT_R_LOW
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
#else
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
#endif
        m_pCanvas = (uint16_t *)malloc(NM3D_WINDOW_WIDTH * NM3D_WINDOW_HEIGHT * 4);
#else
#error "Not supported color pixel mode!"
#endif
        assert(m_pCanvas);
    }
}

CSDLCanvas ::CSDLCanvas()
{
    
}

CSDLCanvas ::~CSDLCanvas()
{
    SDL_DestroyTexture(m_pSDLTexture);
    SDL_DestroyRenderer(m_pSDLRenderer);
    SDL_DestroyWindow(m_pSDLWindow);
    SDL_Quit();
}

void CSDLCanvas ::BitBlt()
{
    // This copies your malloc'd data to the GPU
    SDL_UpdateTexture(m_pSDLTexture, NULL, m_pCanvas, GetCanvasPitch());

    // RENDER
    SDL_RenderClear(m_pSDLRenderer);
    SDL_RenderCopy(m_pSDLRenderer, m_pSDLTexture, NULL, NULL);
    SDL_RenderPresent(m_pSDLRenderer);
}
