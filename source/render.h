// render.h: interface for the CRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(RENDER_H_EC4E5941_DD6A_4781_8CB9_99E1E79DC8A0)
#define RENDER_H_EC4E5941_DD6A_4781_8CB9_99E1E79DC8A0


#include "rgb.h"
#include "font.h"
#include "clipbox.h"
#include "zbuffer.h"
#include "canvas.h"

#ifdef NM3D_PLATFORM_WIN32
#include "../platform/win32/DIB.h"
#endif //NM3D_PLATFORM_WIN32

namespace Nomad3D
{
	class CRender;
	class CMaterial;

	extern CRender render;


#if defined(NM3D_PLATFORM_WIN32)

	inline void MemSet16(void *dest, unsigned short data, int count)
	{
		_asm 
		{ 
			mov edi, dest   ; edi points to destination memory
				mov ecx, count  ; number of 16-bit words to move
				mov ax,  data   ; 16-bit data
				rep stosw       ; move data
		}	
	}
	
	inline void MemSet32(void *dest, unsigned int data, int count)
	{
		_asm 
		{ 
			mov edi, dest   ; edi points to destination memory
				mov ecx, count  ; number of 32-bit words to move
				mov eax, data   ; 32-bit data
				rep stosd       ; move data
		}
	}

#elif defined(NM3D_PLATFORM_LINUX)

	inline void MemSet16(void *dest, uint16_t data, int count)
	{
    	uint16_t *p = (uint16_t *)dest;
    	while (count--)
		{
        	*p++ = data;
    	}
	}

	inline void MemSet32(void *dest, uint32_t data, int count)
	{
    	uint32_t *p = (uint32_t *)dest;
    	while (count--)
		{
        	*p++ = data;
    	}
	}

#else //NM3D_PLATFORM_NDS

	inline void MemSet16(void *dest, unsigned short data, int count)
	{
		unsigned short* pd = (unsigned short*)dest;
		while(count--)
			pd[count]=data;
	}
	
	inline void MemSet32(void *dest, unsigned int data, int count)
	{
		unsigned int* pd = (unsigned int*)dest;
		while(count--)
			pd[count]=data;
	}

#endif

#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
	#define MemSetC	MemSet16 //MemSetC means MemSet Count
#else
	#define MemSetC	MemSet32
#endif

	class CRender  
	{
	public:
		enum ERenderType
		{
			NM3D_RENDER_TYPE_LINE,
			NM3D_RENDER_TYPE_LINE_NO_BACK,
			NM3D_RENDER_TYPE_FLAT,
			NM3D_RENDER_TYPE_GOURAUD,
			NM3D_RENDER_TYPE_GOURAUD_TEXTURE,
			NM3D_RENDER_TYPE_GOURAUD_TEXTURE_SS, // gouraud shading with texture and seprate specular part
			NM3D_RENDER_TYPE_GOURAUD_LaMoth,
			NM3D_RENDER_TYPE_GOURAUD_TEXTURE_LaMoth
		};
#if defined(NM3D_PLATFORM_WIN32)
		void SetDC(HDC dc)
		{
			m_dc = dc;
		}
		CDIB	m_dib;
		HDC		m_dc;
#elif defined(NM3D_PLATFORM_LINUX)
		CCanvas*		m_pCanvas;
#elif defined(NM3D_PLATFORM_NDS)
		unsigned short* frontBuffer = (unsigned short*)(0x06000000);
		unsigned short* backBuffer =  (unsigned short*)(0x06000000 + 256 * 256 * 2);
#else
		#error "Unrecoginized platform or not defined platform"
#endif
	public:
		CRender();
		virtual ~CRender();
	public:
		void SetCanvas(CCanvas* pCanvas);
		void SetRenderType(ERenderType enRT);
		void SetClipBox(const CClipBox& cb);
		void SetClearColor(CRGBA rgba);

		inline void SetColor(const CRGBA rgba[3])
		{
			m_rgba[0] = rgba[0];
			m_rgba[1] = rgba[1];
			m_rgba[2] = rgba[2];
		}
		inline void SetSpecularColor(const CRGBA rgbaSpecular[3])
		{
			m_rgbaSpecular[0] = rgbaSpecular[0];
			m_rgbaSpecular[1] = rgbaSpecular[1];
			m_rgbaSpecular[2] = rgbaSpecular[2];
		}
		inline void SetMaterial(CMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}
		inline void SetTextureCoords(float u[3], float v[3])
		{
			memcpy(m_fUCoords,u,sizeof(float)*3);
			memcpy(m_fVCoords,v,sizeof(float)*3);
		}
		int GetMemPitch();
		inline ERenderType GetRenderType()
		{
			return m_enRenderType;
		}
		//////////////////////////////////////////////////////////////////////////
		void ClearPictureBuffer();
		void ClearVBuffer();
		void DrawTriangle(float poly[3][3]);
		void DrawLine(int x1,int y1,int x2,int y2);
		void Flip();

		//////////////////////////////////////////////////////////////////////////
		void SetZBufferState(bool state=true);
		bool GetZBufferState();
		void SetPictureBGTrans(unsigned char a=0);
		void SetTextColor(CRGBA rgba);
		void DrawText(int x,int y,const char* p);

#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
		inline NM3D_BUFFER GetBuffer()
		{
			return m_pBuff16;
		}
		inline void SetPixel(unsigned short* pbuff16, int y, int x, RGB16 color)
		{
#if defined(NM3D_PLATFORM_NDS)
			*(pbuff16+y<<8+x) = color;
#else
			*(pbuff16+y*NM3D_WINDOW_WIDTH+x) = color;
#endif
		}
		inline void SetPixelA(unsigned short* pbuff16, int y, int x, CRGBA color)
		{
#if defined(NM3D_PLATFORM_NDS)
			unsigned short* p = pbuff16+y<<8+x;
#else
			unsigned short* p = pbuff16+y*NM3D_WINDOW_WIDTH+x;
#endif
			double a = color.a()*0.00390625;//0.00390625=1/256
			CRGB oldcolor(*p);
			CRGB c1 = color*a;
			CRGB c2(oldcolor.rc()*(1-a),oldcolor.gc()*(1-a),oldcolor.bc()*(1-a));
			CRGB c3 = c1+c2;
			*p = c3.GetRGB();
		}
#elif defined(NM3D_PIXELFORMAT_888)
		inline NM3D_BUFFER GetBuffer()
		{
			return m_pBuff32;
		}
		inline void SetPixel(unsigned int* pbuff32, int y, int x, RGB32 color)
		{
#if defined(NM3D_PLATFORM_NDS)
			//ERROR!!!    NDS can not support RGB32 mode!!!!
			*((unsigned short*)pbuff32+y<<8+x) = color;//caution: color is 32bit, not fit to NDS!!!!
#else
			*(pbuff32+y*NM3D_WINDOW_WIDTH+x) = color;
#endif
		}
		inline void SetPixelA(unsigned int* pbuff32, int y, int x, CRGBA color)
		{
#if defined(NM3D_PLATFORM_NDS)
			*((unsigned short*)pbuff32+y<<8+x) = color;//caution: color is 32bit, not compatible with NDS!!!!
#else
			unsigned int* p = pbuff32+y*NM3D_WINDOW_WIDTH+x;
			CRGB oldcolor(*p);
			double a = color.a()*0.00390625;//0.00390625=1/256
			CRGB c1 = color*a;
			CRGB c2(oldcolor.rc()*(1-a),oldcolor.gc()*(1-a),oldcolor.bc()*(1-a));
			CRGB c3 = c1+c2;
			*p = c3.GetRGB();
#endif
		}
#endif

	protected:
		void GouraudShadingZ(float poly[3][3], unsigned short *buff);
		void TextureGouraudShadingZ(float poly[3][3], unsigned short *buff);
		void TextureGouraudShadingSSZ(float poly[3][3], unsigned short *buff);
		void GouraudShading(float poly[3][3], unsigned short *buff);
		void TextureGouraudShading(float poly[3][3], unsigned short *buff);
		void TextureGouraudShadingSS(float poly[3][3], unsigned short *buff);
		void Draw_Triangle_2D3_16(float poly[3][3], unsigned short *buff);
		void Draw_Gouraud_Triangle2_16(float poly[3][3], UCHAR *_dest_buffer);
		void Draw_Textured_TriangleGS_16(float poly[3][3], float uu[3],float vv[3],unsigned short *buff);

		int Cohen_ClipLine(int &x1,int &y1,int &x2, int &y2);
	private:
		ERenderType		m_enRenderType;
		int				m_nMemPitch;
		unsigned char	m_cPictureBGTrans;
		CRGBA			m_rgba[3];
		CRGBA			m_rgbaSpecular[3];
		CRGBA			m_rgbClearColor;
		CMaterial*		m_pMaterial; float m_fUCoords[3]; float m_fVCoords[3];
		unsigned char*	m_pBuff;
		unsigned short* m_pBuff16;
		unsigned int*	m_pBuff32;
		NM3D_CLIP_BOX_VAL

		CFont*			m_pFont;

		bool			m_busezbuffer;
		CZBuffer		m_zbuffer;
	};

	extern CRender render;
}


#endif // !defined(RENDER_H_EC4E5941_DD6A_4781_8CB9_99E1E79DC8A0)
