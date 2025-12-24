

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
#elif defined(NM3D_PIXELFORMAT_555)
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ARGB1555,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
#elif defined(NM3D_PIXELFORMAT888)
#if NM3D_PIXELFORMAT_R_LOW
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
#else
        m_pSDLTexture = SDL_CreateTexture(m_pSDLRenderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING, NM3D_WINDOW_WIDTH, NM3D_WINDOW_HEIGHT);
#endif
#else
#error "Not supported color pixel mode!"
#endif
        m_pCanvas = new NM3D_BUFFER[NM3D_WINDOW_WIDTH * NM3D_WINDOW_HEIGHT];
        assert(m_pCanvas);
    }
}

CSDLCanvas ::CSDLCanvas()
{
    
}

CSDLCanvas ::~CSDLCanvas()
{
    if (m_pCanvas)
    {
        delete m_pCanvas;
        m_pCanvas = NULL;
    }

    SDL_DestroyTexture(m_pSDLTexture);
    SDL_DestroyRenderer(m_pSDLRenderer);
    SDL_DestroyWindow(m_pSDLWindow);
    SDL_Quit();
}

void CSDLCanvas ::BitBlt()
{
    // This copies the CPU side canvas buffer (i.e., m_pCanvas) to the GPU
    SDL_UpdateTexture(m_pSDLTexture, NULL, m_pCanvas, GetCanvasPitch());

    // Render it on GPU
    SDL_RenderClear(m_pSDLRenderer);
    SDL_RenderCopy(m_pSDLRenderer, m_pSDLTexture, NULL, NULL);
    SDL_RenderPresent(m_pSDLRenderer);
}
