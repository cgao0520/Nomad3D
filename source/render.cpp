// render.cpp: implementation of the CRender class.
//
//////////////////////////////////////////////////////////////////////

#include "render.h"
#include "constant.h"
#include "renderlist.h"
#include "font8.h"
#include "util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CRender render;

	CRender::CRender()
	{
		m_nMemPitch = NM3D_WINDOW_WIDTH*sizeof(CRGB);
		m_enRenderType = NM3D_RENDER_TYPE_GOURAUD_TEXTURE;
		min_clip_x = NM3D_CLIP_MIN;
		max_clip_x = NM3D_CLIP_MAX;
		min_clip_y = NM3D_CLIP_MIN;
		max_clip_y = NM3D_CLIP_MAX;

		m_busezbuffer = true;
		m_zbuffer.InitZBuffer(NM3D_WINDOW_WIDTH,NM3D_WINDOW_HEIGHT);
		
#if defined(NM3D_PLATFORM_WIN32)
#if defined(NM3D_PIXELFORMAT_565)
		m_dib.Create(NM3D_WINDOW_WIDTH,-NM3D_WINDOW_HEIGHT,16,DIB_PIXEL_FORMAT16_565);
#elif defined(NM3D_PIXELFORMAT_555)
		m_dib.Create(NM3D_WINDOW_WIDTH,-NM3D_WINDOW_HEIGHT,16/*,DIB_PIXEL_FORMAT16_555*/);
#elif defined(NM3D_PIXELFORMAT888)
		//m_dib.Create
#endif
		m_pBuff = m_dib.GetBits();
#elif defined (NM3D_PLATFORM_LINUX)
		m_pBuff = (unsigned char*)m_canvas.GetCanvas();
#elif defined(NM3D_PLATFORM_NDS)
		//u16* frontBuffer = (u16*)(0x06000000);
		//u16* backBuffer =  (u16*)(0x06000000 + 256 * 256 * 2);
		m_pBuff = (unsigned char*)backBuffer;
#endif
		m_pBuff16 = (unsigned short*)m_pBuff;
		m_pBuff32 = (unsigned int*)m_pBuff;

		m_pFont = new CFont8(m_pBuff);

		m_pMaterial = NULL;//default material is NULL

		m_cPictureBGTrans = 255;
	}

	CRender::~CRender()
	{
		if(m_pFont)
			delete m_pFont;
	}

	void CRender::DrawText(int x,int y,const char* p)
	{
		if(x<0 || y<0)
			m_pFont->DrawString(y,x+min_clip_x,p,GetBuffer());
		else
			m_pFont->DrawString(min_clip_y+y*min_clip_y,x+min_clip_x,p,GetBuffer());
	}
	
	void CRender::SetTextColor(CRGBA rgba)
	{
		m_pFont->SetTextColor(rgba);
	}

	void CRender::SetPictureBGTrans(unsigned char a)
	{
		m_cPictureBGTrans = a;
	}

	void CRender::SetZBufferState(bool state)
	{
		m_busezbuffer = state;
	}

	bool CRender::GetZBufferState()
	{
		return m_busezbuffer;
	}

 	int CRender::GetMemPitch()
 	{
 		return  m_nMemPitch;//NM3D_WINDOW_WIDTH*sizeof(CRGB);
 	}

	void CRender::ClearVBuffer()
	{
		MemSetC(m_pBuff,m_rgbClearColor.GetRGB(),NM3D_WINDOW_WIDTH*NM3D_WINDOW_HEIGHT);

		m_zbuffer.ClearZBuffer();
	}

	void CRender::ClearPictureBuffer()
	{
		int y;
		int width=max_clip_x-min_clip_x+1;
		int mem_pitch = m_nMemPitch>>1;
		int zmem_pitch = NM3D_WINDOW_WIDTH;
		unsigned short* buff_line_x = m_pBuff16+min_clip_y*mem_pitch+min_clip_x;
		int* zbuff_line_x = m_zbuffer.GetBuffer()+min_clip_y*zmem_pitch+min_clip_x;
		if(m_cPictureBGTrans == 0)
		{
			for(y=min_clip_y; y<=max_clip_y; y++)
			{
				MemSet32(zbuff_line_x,NM3D_ZBUFFER_MAX_ZVALUE,width);
				zbuff_line_x += zmem_pitch;
			}
		}
		else
		{
			for(y=min_clip_y; y<=max_clip_y; y++)
			{
				MemSet16(buff_line_x,m_rgbClearColor.GetRGB(),width);
				buff_line_x += mem_pitch;

				MemSet32(zbuff_line_x,NM3D_ZBUFFER_MAX_ZVALUE,width);
				zbuff_line_x += zmem_pitch;
			}
		}
	}

	void CRender::Flip()
	{
#if defined(NM3D_PLATFORM_WIN32)
		m_dib.BitBlt(m_dc,0,0,NM3D_WINDOW_WIDTH,NM3D_WINDOW_HEIGHT,0,0);
#elif defined(NM3D_PLATFORM_LINUX)
		m_canvas.BitBlt();
#elif defined(NM3D_PLATFORM_NDS)
		unsigned char* temp = backBuffer;
		backBuffer = frontBuffer;
		frontBuffer = backBuffer;
#endif
	}

	void CRender::SetClipBox(const CClipBox& cb)
	{
		min_clip_x = cb.min_clip_x;
		max_clip_x = cb.max_clip_x;
		min_clip_y = cb.min_clip_y;
		max_clip_y = cb.max_clip_y;

		if(m_pFont)
			m_pFont->ResetPos(min_clip_y, min_clip_x);
	}

	void CRender::SetClearColor(CRGBA rgba)
	{
		m_rgbClearColor = rgba;
	}

	void CRender::SetRenderType(ERenderType enRT)
	{
		m_enRenderType = enRT;
	}

	void CRender::DrawTriangle(float poly[3][3])
	{
		if(m_busezbuffer)
		{
			switch(m_enRenderType)
			{
			case NM3D_RENDER_TYPE_LINE:
			case NM3D_RENDER_TYPE_LINE_NO_BACK:
				DrawLine(poly[0][0],poly[0][1],poly[1][0],poly[1][1]);
				DrawLine(poly[0][0],poly[0][1],poly[2][0],poly[2][1]);
				DrawLine(poly[1][0],poly[1][1],poly[2][0],poly[2][1]);
				break;
			case NM3D_RENDER_TYPE_FLAT:
				Draw_Triangle_2D3_16(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD:
				GouraudShadingZ(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE:
				TextureGouraudShadingZ(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE_SS:
				TextureGouraudShadingSSZ(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_LaMoth:
				Draw_Gouraud_Triangle2_16(poly,(unsigned char*)m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE_LaMoth:
				Draw_Textured_TriangleGS_16(poly,m_fUCoords,m_fVCoords,m_pBuff16);
				break;
			}
		}
		else
		{
			switch(m_enRenderType)
			{
			case NM3D_RENDER_TYPE_LINE:
			case NM3D_RENDER_TYPE_LINE_NO_BACK:
				DrawLine(poly[0][0],poly[0][1],poly[1][0],poly[1][1]);
				DrawLine(poly[0][0],poly[0][1],poly[2][0],poly[2][1]);
				DrawLine(poly[1][0],poly[1][1],poly[2][0],poly[2][1]);
				break;
			case NM3D_RENDER_TYPE_FLAT:
				Draw_Triangle_2D3_16(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD:
				GouraudShading(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE:
				TextureGouraudShading(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE_SS:
				TextureGouraudShadingSS(poly,m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_LaMoth:
				Draw_Gouraud_Triangle2_16(poly,(unsigned char*)m_pBuff16);
				break;
			case NM3D_RENDER_TYPE_GOURAUD_TEXTURE_LaMoth:
				Draw_Textured_TriangleGS_16(poly,m_fUCoords,m_fVCoords,m_pBuff16);
				break;
			}
		}
	}

	void CRender::DrawLine(int x1,int y1,int x2,int y2)
	{
		if(!Cohen_ClipLine(x1,y1,x2,y2))
			return;
		NM3D_BUFFER buffer=GetBuffer();
		CRGB clr = m_rgba[0];
		int  x, y;
		int  dx, dy;
		int  incx, incy;
		int  balance;
		int i=0;
		if (x2 >= x1)
		{
			dx = x2 - x1;
			incx = 1;
		}
		else
		{
			dx = x1 - x2;
			incx = -1;
		}
		
		if (y2 >= y1)
		{
			dy = y2 - y1;
			incy = 1;
		}
		else
		{
			dy = y1 - y2;
			incy = -1;
		}
		
		x = x1;
		y = y1;
		
		if (dx >= dy)
		{
			dy <<= 1;
			balance = dy - dx;
			dx <<= 1;
			
			while (x != x2)
			{
				SetPixel(buffer,y,x,clr.GetRGB());
				if (balance >= 0)
				{
					y += incy;
					balance -= dx;
				}
				balance += dy;
				x += incx;
				i ++;
			}
			SetPixel(buffer,y,x,clr.GetRGB());
		}
		else
		{
			dx <<= 1;
			balance = dx - dy;
			dy <<= 1;
			
			while (y != y2)
			{
				SetPixel(buffer,y,x,clr.GetRGB());
				if (balance >= 0)
				{
					x += incx;
					balance -= dy;
				}
				balance += dx;
				y += incy;
				i ++;
			}
			SetPixel(buffer,y,x,clr.GetRGB());
		}
	}
	
	int CRender::Cohen_ClipLine(int &x1,int &y1,int &x2, int &y2)
	{
		// this function clips the sent line using the globally defined clipping
		// region
		
		// internal clipping codes
#define CLIP_CODE_C  0x0000
#define CLIP_CODE_N  0x0008
#define CLIP_CODE_S  0x0004
#define CLIP_CODE_E  0x0002
#define CLIP_CODE_W  0x0001
		
#define CLIP_CODE_NE 0x000a
#define CLIP_CODE_SE 0x0006
#define CLIP_CODE_NW 0x0009 
#define CLIP_CODE_SW 0x0005

		
		int xc1=x1, 
			yc1=y1, 
			xc2=x2, 
			yc2=y2;
		
		int p1_code=0, 
			p2_code=0;
		
		// determine codes for p1 and p2
		if (y1 < min_clip_y)
			p1_code|=CLIP_CODE_N;
		else if (y1 > max_clip_y)
			p1_code|=CLIP_CODE_S;
		
		if (x1 < min_clip_x)
			p1_code|=CLIP_CODE_W;
		else if (x1 > max_clip_x)
			p1_code|=CLIP_CODE_E;
		
		if (y2 < min_clip_y)
			p2_code|=CLIP_CODE_N;
		else if (y2 > max_clip_y)
			p2_code|=CLIP_CODE_S;
		
		if (x2 < min_clip_x)
			p2_code|=CLIP_CODE_W;
		else if (x2 > max_clip_x)
			p2_code|=CLIP_CODE_E;
		
		// try and trivially reject
		if ((p1_code & p2_code)) 
			return(0);
		
		// test for totally visible, if so leave points untouched
		if (p1_code==0 && p2_code==0)
			return(1);
		
		// determine end clip point for p1
		switch(p1_code)
		{
		case CLIP_CODE_C: break;
			
		case CLIP_CODE_N:
			{
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
			} break;
		case CLIP_CODE_S:
			{
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);
			} break;
			
		case CLIP_CODE_W:
			{
				xc1 = min_clip_x;
				yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);
			} break;
			
		case CLIP_CODE_E:
			{
				xc1 = max_clip_x;
				yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
			} break;
			
			// these cases are more complex, must compute 2 intersections
		case CLIP_CODE_NE:
			{
				// north hline intersection
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					// east vline intersection
					xc1 = max_clip_x;
					yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if
				
			} break;
			
		case CLIP_CODE_SE:
			{
				// south hline intersection
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					// east vline intersection
					xc1 = max_clip_x;
					yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if
				
			} break;
			
		case CLIP_CODE_NW: 
			{
				// north hline intersection
				yc1 = min_clip_y;
				xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					xc1 = min_clip_x;
					yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
				} // end if
				
			} break;
			
		case CLIP_CODE_SW:
			{
				// south hline intersection
				yc1 = max_clip_y;
				xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
					xc1 = min_clip_x;
					yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
				} // end if
				
			} break;
			
		default:break;
			
		} // end switch
		
		// determine clip point for p2
		switch(p2_code)
		{
		case CLIP_CODE_C: break;
			
		case CLIP_CODE_N:
			{
				yc2 = min_clip_y;
				xc2 = x2 + (min_clip_y-y2)*(x1-x2)/(y1-y2);
			} break;
			
		case CLIP_CODE_S:
			{
				yc2 = max_clip_y;
				xc2 = x2 + (max_clip_y-y2)*(x1-x2)/(y1-y2);
			} break;
			
		case CLIP_CODE_W:
			{
				xc2 = min_clip_x;
				yc2 = y2 + (min_clip_x-x2)*(y1-y2)/(x1-x2);
			} break;
			
		case CLIP_CODE_E:
			{
				xc2 = max_clip_x;
				yc2 = y2 + (max_clip_x-x2)*(y1-y2)/(x1-x2);
			} break;
			
			// these cases are more complex, must compute 2 intersections
		case CLIP_CODE_NE:
			{
				// north hline intersection
				yc2 = min_clip_y;
				xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					// east vline intersection
					xc2 = max_clip_x;
					yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if
				
			} break;
			
		case CLIP_CODE_SE:
			{
				// south hline intersection
				yc2 = max_clip_y;
				xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					// east vline intersection
					xc2 = max_clip_x;
					yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if
				
			} break;
			
		case CLIP_CODE_NW: 
			{
				// north hline intersection
				yc2 = min_clip_y;
				xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					xc2 = min_clip_x;
					yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
				} // end if
				
			} break;
			
		case CLIP_CODE_SW:
			{
				// south hline intersection
				yc2 = max_clip_y;
				xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
				
				// test if intersection is valid, of so then done, else compute next
				if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
					xc2 = min_clip_x;
					yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
				} // end if
				
			} break;
			
		default:break;
			
		} // end switch
		
		// do bounds check
		if ((xc1 < min_clip_x) || (xc1 > max_clip_x) ||
			(yc1 < min_clip_y) || (yc1 > max_clip_y) ||
			(xc2 < min_clip_x) || (xc2 > max_clip_x) ||
			(yc2 < min_clip_y) || (yc2 > max_clip_y) )
		{
			return(0);
		} // end if
		
		// store vars back
		x1 = xc1;
		y1 = yc1;
		x2 = xc2;
		y2 = yc2;
		
		return(1);
		
	} // end Clip_Line

////////////////////////////////////////////////////////////////////////////
// Gouraud Shading start

void CRender::GouraudShading(float poly[3][3], unsigned short *buff)
{
#define GOURAUD_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0);  \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); \
	}while(0)

#ifdef NM3D_PIXELFORMAT_565
	#define GOURAUD_SET_PIXEL	\
	buff_line[x] = ((r >> (NM3D_FIXP16_SHIFT+3)) << 11) + ((g >> (NM3D_FIXP16_SHIFT+2)) << 5) + (b >> (NM3D_FIXP16_SHIFT+3));
	//buff_line[x] = CRGB::GenRGBFixp(r,g,b,NM3D_FIXP16_SHIFT); 
#else
	#define GOURAUD_SET_PIXEL	\
	buff_line[x] = ((r >> (NM3D_FIXP16_SHIFT+3)) << 10) + ((g >> (NM3D_FIXP16_SHIFT+3)) << 5) + (b >> (NM3D_FIXP16_SHIFT+3));
	//buff_line[x] = CRGB::GenRGBFixp(r,g,b,NM3D_FIXP16_SHIFT); 
#endif

	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	// swap, make sure that pt[v1] is on top (minimal y value)
	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;
	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);

	// degenerate triangle
	GOURAUD_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y; int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;

	if(y1==y2 || y1==y3) // flat top
	{
		// swap, make sure pt[v1] is on left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
				SWAP(v1,v2);
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
				SWAP(v1,v3);
		}
		
		// swap, make sure flat top triangle has top left on pt[v1], top right pt[v3] and bottom pt[v2] 
		if(poly[v3][1] > poly[v2][1])
			SWAP(v2,v3);

		GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1);
		m_rgba[v2].GetRGBValuec(r2,g2,b2);
		m_rgba[v3].GetRGBValuec(r3,g3,b3);
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;

		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				buff_line += mem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);
				}
								
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_SET_PIXEL
						
					r += dr_x;
					g += dg_x;
					b += db_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;
				
				buff_line += mem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap, make sure the top vertex is pt[v1], bottom left is pt[v2], bottom right is pt[v3]
		if(poly[v2][0] > poly[v3][0])
			SWAP(v2,v3);
		
		GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1);
		m_rgba[v2].GetRGBValuec(r2,g2,b2);
		m_rgba[v3].GetRGBValuec(r3,g3,b3);
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;
		
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;
				
				buff_line += mem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;
				}
				
				xs += slope_start;
				xe += slope_end;
				
				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;
				
				buff_line += mem_pitch;
			}
		}
	}
	else // arbitrary triangle (not flat one)
	{
		if(poly[v2][1] > poly[v3][1])
			SWAP(v2,v3);

		GOURAUD_GET_POINT_VALUES;
		
		int dy21 = y2 - y1;
		int dy31 = y3 - y1;
		int dy32 = y3 - y2;

		int slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		int slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		int slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		
		int slope_r_v1v2 = (r2-r1)/dy21;
		int slope_g_v1v2 = (g2-g1)/dy21;
		int slope_b_v1v2 = (b2-b1)/dy21;
		int slope_r_v1v3 = (r3-r1)/dy31;
		int slope_g_v1v3 = (g3-g1)/dy31;
		int slope_b_v1v3 = (b3-b1)/dy31;
		int slope_r_v2v3 = (r3-r2)/dy32;
		int slope_g_v2v3 = (g3-g2)/dy32;
		int slope_b_v2v3 = (b3-b2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

		}
		else
		{
			slope_start = slope_v1v3;			
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;
				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				GOURAUD_SET_PIXEL
					
				r += dr_x;
				g += dg_x;
				b += db_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;
			
			buff_line += mem_pitch;
		}

		// draw the lower part (flat top triangle)

		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			int dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = (r1)+slope_r_v1v3*dmy1;
				gs = (g1)+slope_g_v1v3*dmy1;
				bs = (b1)+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = (r2)+slope_r_v2v3*delta_y;
				ge = (g2)+slope_g_v2v3*delta_y;
				be = (b2)+slope_b_v2v3*delta_y;

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;
			}

			ys = min_clip_y;
		}
		else //y2 >= min_clip_y
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = (r1)+slope_r_v1v3*dy21;
				gs = (g1)+slope_g_v1v3*dy21;
				bs = (b1)+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = (r2);
				ge = (g2);
				be = (b2);

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = (r2);
				gs = (g2);
				bs = (b2);
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = (r1)+slope_r_v1v3*dy21;
				ge = (g1)+slope_g_v1v3*dy21;
				be = (b1)+slope_b_v1v3*dy21;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;
				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				GOURAUD_SET_PIXEL
					
				r += dr_x;
				g += dg_x;
				b += db_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;
			
			buff_line += mem_pitch;
		}
	}
}
// Gouraud Shading end

void CRender::GouraudShadingZ(float poly[3][3], unsigned short *buff)
{
#define GOURAUD_Z_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0); z1=(int)(poly[v1][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); z2=(int)(poly[v2][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); z3=(int)(poly[v3][2]*(1<<NM3D_FIXP24_SHIFT)); \
	}while(0)

#ifdef NM3D_PIXELFORMAT_565
	#define GOURAUD_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			buff_line[x] = ((r >> (NM3D_FIXP16_SHIFT+3)) << 11) + ((g >> (NM3D_FIXP16_SHIFT+2)) << 5) + (b >> (NM3D_FIXP16_SHIFT+3)); \
		}
#else
	#define GOURAUD_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			buff_line[x] = ((r >> (NM3D_FIXP16_SHIFT+3)) << 10) + ((g >> (NM3D_FIXP16_SHIFT+3)) << 5) + (b >> (NM3D_FIXP16_SHIFT+3)); \
		}
#endif

	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	//Z Buffer Variables
	int* zbuff=m_zbuffer.GetBuffer();
	int* zbuff_line=zbuff;
	int zmem_pitch = NM3D_WINDOW_WIDTH;
	int z1,z2,z3;
	int slope_start_z,slope_end_z;
	int z,zs,ze;
	int dz_x;
	
	// swap to make sure pt[v1] is on top (minimal y value)
	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;
	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);

	// degenerate triangle
	GOURAUD_Z_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y; int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;

	if(y1==y2 || y1==y3) // flat top
	{
		// swap to make sure pt[v1] is left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
				SWAP(v1,v2);
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
				SWAP(v1,v3);
		}
		
		// swap to make sure the flat top triangle has top left pt[v1], top right pt[v3], and bottom pt[v2]
		if(poly[v3][1] > poly[v2][1])
			SWAP(v2,v3);

		GOURAUD_Z_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1);
		m_rgba[v2].GetRGBValuec(r2,g2,b2);
		m_rgba[v3].GetRGBValuec(r3,g3,b3);
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;


		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z2-z3)/dy;

		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z3 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z3;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;

				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;
					
					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;

				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;

				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Z Buffer
					dz_x = (ze-zs);
				}
								
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_Z_SET_PIXEL
						
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap to make sure top vertex is pt[v1], bottom left pt[v2], and bottom right pt[v3]
		if(poly[v2][0] > poly[v3][0])
			SWAP(v2,v3);
		
		GOURAUD_Z_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1);
		m_rgba[v2].GetRGBValuec(r2,g2,b2);
		m_rgba[v3].GetRGBValuec(r3,g3,b3);
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;

		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z3-z1)/dy;
		
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					GOURAUD_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;
				
				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else // arbitrary trianble
	{
		if(poly[v2][1] > poly[v3][1])
			SWAP(v2,v3);

		GOURAUD_Z_GET_POINT_VALUES;

		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		
		int dy21 = y2 - y1;
		int dy31 = y3 - y1;
		int dy32 = y3 - y2;

		int slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		int slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		int slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		int slope_r_v1v2 = (r2-r1)/dy21;
		int slope_g_v1v2 = (g2-g1)/dy21;
		int slope_b_v1v2 = (b2-b1)/dy21;
		int slope_r_v1v3 = (r3-r1)/dy31;
		int slope_g_v1v3 = (g3-g1)/dy31;
		int slope_b_v1v3 = (b3-b1)/dy31;
		int slope_r_v2v3 = (r3-r2)/dy32;
		int slope_g_v2v3 = (g3-g2)/dy32;
		int slope_b_v2v3 = (b3-b2)/dy32;
		int slope_z_v1v2 = (z2-z1)/dy21;
		int slope_z_v1v3 = (z3-z1)/dy31;
		int slope_z_v2v3 = (z3-z2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

			// Z Buffer
			slope_start_z = slope_z_v1v2;
			slope_end_z = slope_z_v1v3;
		}
		else
		{
			slope_start = slope_v1v3;			
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;

			// Z Buffer
			slope_start_z = slope_z_v1v3;
			slope_end_z = slope_z_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Z Buffer
				dz_x = (ze-zs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				GOURAUD_Z_SET_PIXEL
					
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;
			
			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}

		// draw the lower part (flat top triangle)

		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			int dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = (r1)+slope_r_v1v3*dmy1;
				gs = (g1)+slope_g_v1v3*dmy1;
				bs = (b1)+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = (r2)+slope_r_v2v3*delta_y;
				ge = (g2)+slope_g_v2v3*delta_y;
				be = (b2)+slope_b_v2v3*delta_y;

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dmy1;
				ze = z2+slope_z_v2v3*delta_y;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Z Buffer
				zs = z2+slope_z_v2v3*delta_y;
				ze = z1+slope_z_v1v3*dmy1;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = min_clip_y;
		}
		else //y2 >= min_clip_y
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = (r1)+slope_r_v1v3*dy21;
				gs = (g1)+slope_g_v1v3*dy21;
				bs = (b1)+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = (r2);
				ge = (g2);
				be = (b2);

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dy21;
				ze = z2;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = (r2);
				gs = (g2);
				bs = (b2);
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = (r1)+slope_r_v1v3*dy21;
				ge = (g1)+slope_g_v1v3*dy21;
				be = (b1)+slope_b_v1v3*dy21;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Z Buffer
				zs = z2;
				ze = z1+slope_z_v1v3*dy21;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Z Buffer
				dz_x = (ze-zs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				GOURAUD_Z_SET_PIXEL
					
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;
			
			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// Texture Gouraud Shading start

void CRender::TextureGouraudShading(float poly[3][3], unsigned short *buff)
{
#define TEXTURE_GOURAUD_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0);  \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); \
	}while(0)


#ifdef NM3D_PIXELFORMAT_565 
	#define TEXTURE_GOURAUD_SET_PIXEL	\
		buff_line[x] = ((rr >> (NM3D_FIXP16_SHIFT+8)) << 11) + ((gg >> (NM3D_FIXP16_SHIFT+8)) << 5) + (bb >> (NM3D_FIXP16_SHIFT+8)); 
#else 
	#define TEXTURE_GOURAUD_SET_PIXEL	\
		buff_line[x] = ((rr >> (NM3D_FIXP16_SHIFT+8)) << 10) + ((gg >> (NM3D_FIXP16_SHIFT+8)) << 5) + (bb >> (NM3D_FIXP16_SHIFT+8));
#endif

	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;
	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y;int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;
//--start the following variables used by general triangle
	int dy21;
	int dy31;
	int dy32;
	
	int slope_v1v2;
	int slope_v1v3;
	int slope_v2v3;
	
	int slope_r_v1v2;
	int slope_g_v1v2;
	int slope_b_v1v2;
	int slope_r_v1v3;
	int slope_g_v1v3;
	int slope_b_v1v3;
	int slope_r_v2v3;
	int slope_g_v2v3;
	int slope_b_v2v3;
	
	int slope_u_v1v2;
	int slope_v_v1v2;
	int slope_u_v1v3;
	int slope_v_v1v3;
	int slope_u_v2v3;
	int slope_v_v2v3;
	int dmy1;
//--end the above variables used by general triangle

	// U,V
	int _u1,_u2,_u3,_v1,_v2,_v3;
	int slope_start_u,slope_start_v;
	int slope_end_u,slope_end_v;
	int du_x,dv_x;
	int us,ue,vs,ve;
	int u,v;
	unsigned int rr,gg,bb;
	CRGB* texture = (CRGB*)(m_pMaterial->m_imgTexture.GetData());
	unsigned char texture_width_log = Log2(m_pMaterial->m_imgTexture.GetWidth());	

	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	// swap to make sure pt[v1] is on top (minimal y value)
	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);

	// degenerate triangle
	TEXTURE_GOURAUD_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	if(!texture)
	{
		GouraudShading(poly,buff);
		return;
	}

	if(y1==y2 || y1==y3) // flat top
	{
		// swap to make sure pt[v1] is left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
			{
				SWAP(v1,v2);
				SWAP(x1,x2);
				SWAP(y1,y2);
			}
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
			{
				SWAP(v1,v3);
				SWAP(x1,x3);
				SWAP(y1,y3);
			}
		}
		
		// swap to make sure the flat top triangle with pt[v1] on top left, pt[v3] top right, and pv[v2] at bottom
		if(poly[v3][1] > poly[v2][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}

		//TEXTURE_GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u2-_u3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v2-_v3)<<NM3D_FIXP16_SHIFT)/dy;

		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u3<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v3<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u3<<NM3D_FIXP16_SHIFT);
			ve = (_v3<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
				
					TEXTURE_GOURAUD_SET_PIXEL
								
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
		else // no x clip
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;

					TEXTURE_GOURAUD_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap to make sure pt[v1] on top, pt[v2] at bottom left, and pt[v3] bottom right
		if(poly[v2][0] > poly[v3][0])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}
		
		//TEXTURE_GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u3-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v3-_v1)<<NM3D_FIXP16_SHIFT)/dy;
	
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);
			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u1<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v1<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u1<<NM3D_FIXP16_SHIFT);
			ve = (_v1<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
	}
	else // arbitrary triangle
	{
		if(poly[v2][1] > poly[v3][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}

		//TEXTURE_GOURAUD_GET_POINT_VALUES;

		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;

		_u1 = (((int)(m_fUCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_u2 = (((int)(m_fUCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_u3 = (((int)(m_fUCoords[v3]))<<NM3D_FIXP16_SHIFT);
		_v1 = (((int)(m_fVCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_v2 = (((int)(m_fVCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_v3 = (((int)(m_fVCoords[v3]))<<NM3D_FIXP16_SHIFT);

		// OK now
		
		dy21 = y2 - y1;
		dy31 = y3 - y1;
		dy32 = y3 - y2;

		slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		slope_r_v1v2 = (r2-r1)/dy21;
		slope_g_v1v2 = (g2-g1)/dy21;
		slope_b_v1v2 = (b2-b1)/dy21;
		slope_r_v1v3 = (r3-r1)/dy31;
		slope_g_v1v3 = (g3-g1)/dy31;
		slope_b_v1v3 = (b3-b1)/dy31;
		slope_r_v2v3 = (r3-r2)/dy32;
		slope_g_v2v3 = (g3-g2)/dy32;
		slope_b_v2v3 = (b3-b2)/dy32;

		slope_u_v1v2 = (_u2-_u1)/dy21;
		slope_v_v1v2 = (_v2-_v1)/dy21;
		slope_u_v1v3 = (_u3-_u1)/dy31;
		slope_v_v1v3 = (_v3-_v1)/dy31;
		slope_u_v2v3 = (_u3-_u2)/dy32;
		slope_v_v2v3 = (_v3-_v2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

			slope_start_u = slope_u_v1v2;
			slope_start_v = slope_v_v1v2;
			slope_end_u = slope_u_v1v3;
			slope_end_v = slope_v_v1v3;
		}
		else
		{
			slope_start = slope_v1v3;			
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;

			slope_start_u = slope_u_v1v3;
			slope_start_v = slope_v_v1v3;
			slope_end_u = slope_u_v1v2;
			slope_end_v = slope_v_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			us = _u1 + slope_start_u*delta_y;
			vs = _v1 + slope_start_v*delta_y;
			ue = _u1 + slope_end_u*delta_y;
			ve = _v1 + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			us = _u1;
			vs = _v1;
			ue = _u1;
			ve = _v1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<=ye-1; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				du_x = (ue-us);
				dv_x = (ve-vs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;

			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				u += du_x;
				v += dv_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			buff_line += mem_pitch;
		}

		// draw the lower part (flat top triangle)
		
		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = r1+slope_r_v1v3*dmy1;
				gs = g1+slope_g_v1v3*dmy1;
				bs = b1+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = r2+slope_r_v2v3*delta_y;
				ge = g2+slope_g_v2v3*delta_y;
				be = b2+slope_b_v2v3*delta_y;

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				us = _u1 + slope_u_v1v3*dmy1;
				vs = _v1 + slope_v_v1v3*dmy1;
				ue = _u2 + slope_u_v2v3*delta_y;
				ve = _v2 + slope_v_v2v3*delta_y;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				us = _u2 + slope_u_v2v3*delta_y;
				vs = _v2 + slope_v_v2v3*delta_y;
				ue = _u1 + slope_u_v1v3*dmy1;
				ve = _v1 + slope_v_v1v3*dmy1;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;
			}

			ys = min_clip_y;
		}
		else //y2 >= min_clip_y
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = r1+slope_r_v1v3*dy21;
				gs = g1+slope_g_v1v3*dy21;
				bs = b1+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = r2;
				ge = g2;
				be = b2;
				
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				us = _u1+slope_u_v1v3*dy21;
				vs = _v1+slope_v_v1v3*dy21;
				ue = _u2;
				ve = _v2;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = r2;
				gs = g2;
				bs = b2;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = r1+slope_r_v1v3*dy21;
				ge = g1+slope_g_v1v3*dy21;
				be = b1+slope_b_v1v3*dy21;
				
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				us = _u2;
				vs = _v2;
				ue = _u1+slope_u_v1v3*dy21;
				ve = _v1+slope_v_v1v3*dy21;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				du_x = (ue-us);
				dv_x = (ve-vs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;
				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				u += du_x;
				v += dv_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			buff_line += mem_pitch;
		}
	}
}

void CRender::TextureGouraudShadingZ(float poly[3][3], unsigned short *buff)
{
#define TEXTURE_GOURAUD_Z_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0); z1=(int)(poly[v1][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); z2=(int)(poly[v2][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); z3=(int)(poly[v3][2]*(1<<NM3D_FIXP24_SHIFT)); \
	}while(0)


#ifdef NM3D_PIXELFORMAT_565 
	#define TEXTURE_GOURAUD_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			buff_line[x] = ((rr >> (NM3D_FIXP16_SHIFT+8)) << 11) + ((gg >> (NM3D_FIXP16_SHIFT+8)) << 5) + (bb >> (NM3D_FIXP16_SHIFT+8)); \
		}
#else 
	#define TEXTURE_GOURAUD_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			buff_line[x] = ((rr >> (NM3D_FIXP16_SHIFT+8)) << 10) + ((gg >> (NM3D_FIXP16_SHIFT+8)) << 5) + (bb >> (NM3D_FIXP16_SHIFT+8)); \
		}
#endif

	//Z Buffer Variables
	int* zbuff=m_zbuffer.GetBuffer();
	int* zbuff_line=zbuff;
	int zmem_pitch = NM3D_WINDOW_WIDTH;
	int z1,z2,z3;
	int slope_start_z,slope_end_z;
	int z,zs,ze;
	int dz_x;

	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;
	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y;int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;
//--start the following variables used by general triangle
	int dy21;
	int dy31;
	int dy32;
	
	int slope_v1v2;
	int slope_v1v3;
	int slope_v2v3;
	
	int slope_r_v1v2;
	int slope_g_v1v2;
	int slope_b_v1v2;
	int slope_r_v1v3;
	int slope_g_v1v3;
	int slope_b_v1v3;
	int slope_r_v2v3;
	int slope_g_v2v3;
	int slope_b_v2v3;
	
	int slope_u_v1v2;
	int slope_v_v1v2;
	int slope_u_v1v3;
	int slope_v_v1v3;
	int slope_u_v2v3;
	int slope_v_v2v3;
	int dmy1;

	int slope_z_v1v2;
	int slope_z_v1v3;
	int slope_z_v2v3;

//--end the above variables used by general triangle

	// U,V
	int _u1,_u2,_u3,_v1,_v2,_v3;
	int slope_start_u,slope_start_v;
	int slope_end_u,slope_end_v;
	int du_x,dv_x;
	int us,ue,vs,ve;
	int u,v;
	unsigned int rr,gg,bb;
	CRGB* texture = (CRGB*)(m_pMaterial->m_imgTexture.GetData());
	unsigned char texture_width_log = Log2(m_pMaterial->m_imgTexture.GetWidth());	

	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	// swap to make sure pt[v1] is on top (minimal y value)
	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);

	// degenerate triangle
	TEXTURE_GOURAUD_Z_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	if(!texture)
	{
		GouraudShadingZ(poly,buff);
		return;
	}

	if(y1==y2 || y1==y3) // flat top
	{
		// swap to make sure pt[v1] is left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
			{
				SWAP(v1,v2);
				SWAP(x1,x2);
				SWAP(y1,y2);
				SWAP(z1,z2);
			}
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
			{
				SWAP(v1,v3);
				SWAP(x1,x3);
				SWAP(y1,y3);
				SWAP(z1,z3);
			}
		}
		
		// swap to make sure the flat top trianglehas pt[v1] on top left, pt[v3] top right, and pt[v2] at bottom
		if(poly[v3][1] > poly[v2][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}

		//TEXTURE_GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u2-_u3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v2-_v3)<<NM3D_FIXP16_SHIFT)/dy;

		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z2-z3)/dy;


		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u3<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v3<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z3 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u3<<NM3D_FIXP16_SHIFT);
			ve = (_v3<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z3;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;
			
			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
				
					TEXTURE_GOURAUD_Z_SET_PIXEL
								
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else // no x clip
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;

					TEXTURE_GOURAUD_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap to make sure that the flat bottom triangle has pt[v1] on top, pt[v2] bottom left, and pt[v3] bottom right
		if(poly[v2][0] > poly[v3][0])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}
		
		//TEXTURE_GOURAUD_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u3-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v3-_v1)<<NM3D_FIXP16_SHIFT)/dy;

		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z3-z1)/dy;
	
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);
			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u1<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v1<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u1<<NM3D_FIXP16_SHIFT);
			ve = (_v1<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_Z_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else // arbitrary triangle
	{
		if(poly[v2][1] > poly[v3][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}

		//TEXTURE_GOURAUD_GET_POINT_VALUES;

		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;

		_u1 = (((int)(m_fUCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_u2 = (((int)(m_fUCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_u3 = (((int)(m_fUCoords[v3]))<<NM3D_FIXP16_SHIFT);
		_v1 = (((int)(m_fVCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_v2 = (((int)(m_fVCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_v3 = (((int)(m_fVCoords[v3]))<<NM3D_FIXP16_SHIFT);

		// OK now
		
		dy21 = y2 - y1;
		dy31 = y3 - y1;
		dy32 = y3 - y2;

		slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		slope_r_v1v2 = (r2-r1)/dy21;
		slope_g_v1v2 = (g2-g1)/dy21;
		slope_b_v1v2 = (b2-b1)/dy21;
		slope_r_v1v3 = (r3-r1)/dy31;
		slope_g_v1v3 = (g3-g1)/dy31;
		slope_b_v1v3 = (b3-b1)/dy31;
		slope_r_v2v3 = (r3-r2)/dy32;
		slope_g_v2v3 = (g3-g2)/dy32;
		slope_b_v2v3 = (b3-b2)/dy32;

		slope_u_v1v2 = (_u2-_u1)/dy21;
		slope_v_v1v2 = (_v2-_v1)/dy21;
		slope_u_v1v3 = (_u3-_u1)/dy31;
		slope_v_v1v3 = (_v3-_v1)/dy31;
		slope_u_v2v3 = (_u3-_u2)/dy32;
		slope_v_v2v3 = (_v3-_v2)/dy32;

		// Z Buffer
		slope_z_v1v2 = (z2-z1)/dy21;
		slope_z_v1v3 = (z3-z1)/dy31;
		slope_z_v2v3 = (z3-z2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

			slope_start_u = slope_u_v1v2;
			slope_start_v = slope_v_v1v2;
			slope_end_u = slope_u_v1v3;
			slope_end_v = slope_v_v1v3;

			// Z Buffer
			slope_start_z = slope_z_v1v2;
			slope_end_z = slope_z_v1v3;
		}
		else
		{
			slope_start = slope_v1v3;			
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;

			slope_start_u = slope_u_v1v3;
			slope_start_v = slope_v_v1v3;
			slope_end_u = slope_u_v1v2;
			slope_end_v = slope_v_v1v2;

			// Z Buffer
			slope_start_z = slope_z_v1v3;
			slope_end_z = slope_z_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			us = _u1 + slope_start_u*delta_y;
			vs = _v1 + slope_start_v*delta_y;
			ue = _u1 + slope_end_u*delta_y;
			ve = _v1 + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			us = _u1;
			vs = _v1;
			ue = _u1;
			ve = _v1;

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<=ye-1; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				du_x = (ue-us);
				dv_x = (ve-vs);

				// Z Buffer
				dz_x = (ze-zs);
			}
			
			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;

			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_Z_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				u += du_x;
				v += dv_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;

			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}

		// draw the lower part (flat top triangle)
		
		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = r1+slope_r_v1v3*dmy1;
				gs = g1+slope_g_v1v3*dmy1;
				bs = b1+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = r2+slope_r_v2v3*delta_y;
				ge = g2+slope_g_v2v3*delta_y;
				be = b2+slope_b_v2v3*delta_y;

				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				us = _u1 + slope_u_v1v3*dmy1;
				vs = _v1 + slope_v_v1v3*dmy1;
				ue = _u2 + slope_u_v2v3*delta_y;
				ve = _v2 + slope_v_v2v3*delta_y;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dmy1;
				ze = z2+slope_z_v2v3*delta_y;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;

				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				us = _u2 + slope_u_v2v3*delta_y;
				vs = _v2 + slope_v_v2v3*delta_y;
				ue = _u1 + slope_u_v1v3*dmy1;
				ve = _v1 + slope_v_v1v3*dmy1;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;

				// Z Buffer
				zs = z2+slope_z_v2v3*delta_y;
				ze = z1+slope_z_v1v3*dmy1;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = min_clip_y;
		}
		else //y2 >= min_clip_y
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = r1+slope_r_v1v3*dy21;
				gs = g1+slope_g_v1v3*dy21;
				bs = b1+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = r2;
				ge = g2;
				be = b2;
				
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				us = _u1+slope_u_v1v3*dy21;
				vs = _v1+slope_v_v1v3*dy21;
				ue = _u2;
				ve = _v2;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dy21;
				ze = z2;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = r2;
				gs = g2;
				bs = b2;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = r1+slope_r_v1v3*dy21;
				ge = g1+slope_g_v1v3*dy21;
				be = b1+slope_b_v1v3*dy21;
				
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				us = _u2;
				vs = _v2;
				ue = _u1+slope_u_v1v3*dy21;
				ve = _v1+slope_v_v1v3*dy21;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;

				// Z Buffer
				zs = z2;
				ze = z1+slope_z_v1v3*dy21;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				du_x = (ue-us);
				dv_x = (ve-vs);

				// Z Buffer
				dz_x = (ze-zs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_Z_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				u += du_x;
				v += dv_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;

			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//TextureGouraudShadingSS

void CRender::TextureGouraudShadingSS(float poly[3][3], unsigned short *buff)
{
#define TEXTURE_GOURAUD_SS_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0);  \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); \
	}while(0)

/*
#if (NM3D_LIGHT_SEPERATE_SPECULAR)
	#define TEXTURE_GOURAUD_ADD_SPECULAR_PART	\
		r += m_rgbSpecular[]
*/

#ifdef NM3D_PIXELFORMAT_565 
	#define TEXTURE_GOURAUD_SS_SET_PIXEL	\
		rr += (sr<<5); \
		gg += (sg<<6); \
		bb += (sb<<5); \
		rr = rr>>(NM3D_FIXP16_SHIFT+5); rr = MIN(rr,255); \
		gg = gg>>(NM3D_FIXP16_SHIFT+6); gg = MIN(gg,255); \
		bb = bb>>(NM3D_FIXP16_SHIFT+5); bb = MIN(bb,255); \
		buff_line[x] = CRGB(rr,gg,bb).GetRGB();
#else 
	#define TEXTURE_GOURAUD_SS_SET_PIXEL	\
		rr += (sr<<5); \
		gg += (sg<<5); \
		bb += (sb<<5); \
		rr = rr>>(NM3D_FIXP16_SHIFT+5); rr = MIN(rr,255); \
		gg = gg>>(NM3D_FIXP16_SHIFT+5); gg = MIN(gg,255); \
		bb = bb>>(NM3D_FIXP16_SHIFT+5); bb = MIN(bb,255); \
		buff_line[x] = CRGB(rr,gg,bb).GetRGB();
#endif

	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	// swap to make sure pt[v1] is on top (minimal y value)
	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;
	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);

	// degenerate triangle
	TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y;int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
//	int dx_start=0,dx_end=0;
//	int dr_start=0,dg_start=0,db_start=0;
//	int dr_end=0,dg_end=0,db_end=0;
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;

	//--start the following variables used by general triangle
	int dy21;
	int dy31;
	int dy32;
	
	int slope_v1v2;
	int slope_v1v3;
	int slope_v2v3;
	
	int slope_r_v1v2;
	int slope_g_v1v2;
	int slope_b_v1v2;
	int slope_r_v1v3;
	int slope_g_v1v3;
	int slope_b_v1v3;
	int slope_r_v2v3;
	int slope_g_v2v3;
	int slope_b_v2v3;
	
	// Separate Specular Part
	int slope_sr_v1v2;
	int slope_sg_v1v2;
	int slope_sb_v1v2;
	int slope_sr_v1v3;
	int slope_sg_v1v3;
	int slope_sb_v1v3;
	int slope_sr_v2v3;
	int slope_sg_v2v3;
	int slope_sb_v2v3;
	
	int slope_u_v1v2;
	int slope_v_v1v2;
	int slope_u_v1v3;
	int slope_v_v1v3;
	int slope_u_v2v3;
	int slope_v_v2v3;
	int dmy1;
	//--end the above variables used by general triangle

	// Separate Specular Part
	int slope_start_sr,slope_start_sg,slope_start_sb;
	int slope_end_sr,slope_end_sg,slope_end_sb;
	int sr1,sg1,sb1,sr2,sg2,sb2,sr3,sg3,sb3;
//	int dsr_start=0,dsg_start=0,dsb_start=0;
//	int dsr_end=0,dsg_end=0,dsb_end=0;
	int srs,sre,sgs,sge,sbs,sbe;
	int dsr_x,dsg_x,dsb_x;
	int sr,sg,sb;

	// U,V
	int _u1,_u2,_u3,_v1,_v2,_v3;
	int slope_start_u,slope_start_v;
	int slope_end_u,slope_end_v;
//	int du_start=0,dv_start=0;
//	int du_end=0,dv_end=0;
	int du_x,dv_x;
	int us,ue,vs,ve;
	int u,v;
	unsigned int rr,gg,bb;
	CRGB* texture = (CRGB*)(m_pMaterial->m_imgTexture.GetData());
	unsigned char texture_width_log = Log2(m_pMaterial->m_imgTexture.GetWidth());
	if(!texture)
	{
		GouraudShading(poly,buff);
		return;
	}
	

	if(y1==y2 || y1==y3) // flat top
	{
		// swap to make sure pt[v1] is left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
			{
				SWAP(v1,v2);
				SWAP(x1,x2);
				SWAP(y1,y2);
			}
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
			{
				SWAP(v1,v3);
				SWAP(x1,x3);
				SWAP(y1,y3);
			}
		}
		
		// swap to make sure the flat top triangle has pt[v1] on top left, pt[v3] top right, and pt[v2] at bottom
		if(poly[v3][1] > poly[v2][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}

		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1);// sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2);// sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3);// sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;
		slope_start_sr = ((sr2-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sg = ((sg2-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sb = ((sb2-sb1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sr = ((sr2-sr3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sg = ((sg2-sg3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sb = ((sb2-sb3)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u2-_u3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v2-_v3)<<NM3D_FIXP16_SHIFT)/dy;

		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT) + slope_start_sr*delta_y;
			sgs = (sg1<<NM3D_FIXP16_SHIFT) + slope_start_sg*delta_y;
			sbs = (sb1<<NM3D_FIXP16_SHIFT) + slope_start_sb*delta_y;
			sre = (sr3<<NM3D_FIXP16_SHIFT) + slope_end_sr*delta_y;
			sge = (sg3<<NM3D_FIXP16_SHIFT) + slope_end_sg*delta_y;
			sbe = (sb3<<NM3D_FIXP16_SHIFT) + slope_end_sb*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u3<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v3<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT);
			sgs = (sg1<<NM3D_FIXP16_SHIFT);
			sbs = (sb1<<NM3D_FIXP16_SHIFT);
			sre = (sr3<<NM3D_FIXP16_SHIFT);
			sge = (sg3<<NM3D_FIXP16_SHIFT);
			sbe = (sb3<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u3<<NM3D_FIXP16_SHIFT);
			ve = (_v3<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Separate Specular Part
					sr += dsr_x*delta_x;
					sg += dsg_x*delta_x;
					sb += dsb_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
				
					TEXTURE_GOURAUD_SS_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
		else // no x clip
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_SET_PIXEL
						
					r += dr_x;
					g += dg_x;
					b += db_x;
					
					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;
					
					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;
				
				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;
				
				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;
				
				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap to make sure the flat top triangle has pt[v1] on top, pt[v2] bottom left, and pt[v3] bottom right
		if(poly[v2][0] > poly[v3][0])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}
		
		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1);// r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2);// r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3);// r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1);// sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2);// sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3);// sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;
		slope_start_sr = ((sr2-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sg = ((sg2-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sb = ((sb2-sb1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sr = ((sr3-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sg = ((sg3-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sb = ((sb3-sb1)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u3-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v3-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT) + slope_start_sr*delta_y;
			sgs = (sg1<<NM3D_FIXP16_SHIFT) + slope_start_sg*delta_y;
			sbs = (sb1<<NM3D_FIXP16_SHIFT) + slope_start_sb*delta_y;
			sre = (sr1<<NM3D_FIXP16_SHIFT) + slope_end_sr*delta_y;
			sge = (sg1<<NM3D_FIXP16_SHIFT) + slope_end_sg*delta_y;
			sbe = (sb1<<NM3D_FIXP16_SHIFT) + slope_end_sb*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u1<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v1<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT);
			sgs = (sg1<<NM3D_FIXP16_SHIFT);
			sbs = (sb1<<NM3D_FIXP16_SHIFT);
			sre = (sr1<<NM3D_FIXP16_SHIFT);
			sge = (sg1<<NM3D_FIXP16_SHIFT);
			sbe = (sb1<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u1<<NM3D_FIXP16_SHIFT);
			ve = (_v1<<NM3D_FIXP16_SHIFT);

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;

				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Separate Specular Part
					sr += dsr_x*delta_x;
					sg += dsg_x*delta_x;
					sb += dsb_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;
					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;

				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;
				
				buff_line += mem_pitch;
			}
		}
	}
	else // arbitrary triangle
	{
		if(poly[v2][1] > poly[v3][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
		}

		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1); sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2); sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3); sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;

		_u1 = (((int)(m_fUCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_u2 = (((int)(m_fUCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_u3 = (((int)(m_fUCoords[v3]))<<NM3D_FIXP16_SHIFT);
		_v1 = (((int)(m_fVCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_v2 = (((int)(m_fVCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_v3 = (((int)(m_fVCoords[v3]))<<NM3D_FIXP16_SHIFT);

		dy21 = y2 - y1;
		dy31 = y3 - y1;
		dy32 = y3 - y2;
		
		slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		slope_r_v1v2 = (r2-r1)/dy21;
		slope_g_v1v2 = (g2-g1)/dy21;
		slope_b_v1v2 = (b2-b1)/dy21;
		slope_r_v1v3 = (r3-r1)/dy31;
		slope_g_v1v3 = (g3-g1)/dy31;
		slope_b_v1v3 = (b3-b1)/dy31;
		slope_r_v2v3 = (r3-r2)/dy32;
		slope_g_v2v3 = (g3-g2)/dy32;
		slope_b_v2v3 = (b3-b2)/dy32;

		// Separate Specular Part
		slope_sr_v1v2 = (sr2-sr1)/dy21;
		slope_sg_v1v2 = (sg2-sg1)/dy21;
		slope_sb_v1v2 = (sb2-sb1)/dy21;
		slope_sr_v1v3 = (sr3-sr1)/dy31;
		slope_sg_v1v3 = (sg3-sg1)/dy31;
		slope_sb_v1v3 = (sb3-sb1)/dy31;
		slope_sr_v2v3 = (sr3-sr2)/dy32;
		slope_sg_v2v3 = (sg3-sg2)/dy32;
		slope_sb_v2v3 = (sb3-sb2)/dy32;

		slope_u_v1v2 = (_u2-_u1)/dy21;
		slope_v_v1v2 = (_v2-_v1)/dy21;
		slope_u_v1v3 = (_u3-_u1)/dy31;
		slope_v_v1v3 = (_v3-_v1)/dy31;
		slope_u_v2v3 = (_u3-_u2)/dy32;
		slope_v_v2v3 = (_v3-_v2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

			// Separate Specular Part
			slope_start_sr = slope_sr_v1v2;
			slope_start_sg = slope_sg_v1v2;
			slope_start_sb = slope_sb_v1v2;
			slope_end_sr = slope_sr_v1v3;
			slope_end_sg = slope_sg_v1v3;
			slope_end_sb = slope_sb_v1v3;

			slope_start_u = slope_u_v1v2;
			slope_start_v = slope_v_v1v2;
			slope_end_u = slope_u_v1v3;
			slope_end_v = slope_v_v1v3;
		}
		else
		{
			slope_start = slope_v1v3;
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;

			// Separate Specular Part
			slope_start_sr = slope_sr_v1v3;
			slope_start_sg = slope_sg_v1v3;
			slope_start_sb = slope_sb_v1v3;
			slope_end_sr = slope_sr_v1v2;
			slope_end_sg = slope_sg_v1v2;
			slope_end_sb = slope_sb_v1v2;

			slope_start_u = slope_u_v1v3;
			slope_start_v = slope_v_v1v3;
			slope_end_u = slope_u_v1v2;
			slope_end_v = slope_v_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			// Separate Specular Part
			srs = sr1 + slope_start_sr*delta_y;
			sgs = sg1 + slope_start_sg*delta_y;
			sbs = sb1 + slope_start_sb*delta_y;
			sre = sr1 + slope_end_sr*delta_y;
			sge = sg1 + slope_end_sg*delta_y;
			sbe = sb1 + slope_end_sb*delta_y;

			us = _u1 + slope_start_u*delta_y;
			vs = _v1 + slope_start_v*delta_y;
			ue = _u1 + slope_end_u*delta_y;
			ve = _v1 + slope_end_v*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			// Separate Specular Part
			srs = sr1;
			sgs = sg1;
			sbs = sb1;
			sre = sr1;
			sge = sg1;
			sbe = sb1;
			
			us = _u1;
			vs = _v1;
			ue = _u1;
			ve = _v1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<=ye-1; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			// Separate Specular Part
			sr=srs + NM3D_FIXP16_ROUND_UP;
			sg=sgs + NM3D_FIXP16_ROUND_UP;
			sb=sbs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Separate Specular Part
				dsr_x = (sre-srs)/dx;
				dsg_x = (sge-sgs)/dx;
				dsb_x = (sbe-sbs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Separate Specular Part
				dsr_x = (sre-srs);
				dsg_x = (sge-sgs);
				dsb_x = (sbe-sbs);

				du_x = (ue-us);
				dv_x = (ve-vs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Separate Specular Part
				sr += dsr_x*delta_x;
				sg += dsg_x*delta_x;
				sb += dsb_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;

			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SS_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Separate Specular Part
				sr += dsr_x;
				sg += dsg_x;
				sb += dsb_x;

				u += du_x;
				v += dv_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Separate Specular Part
			srs += slope_start_sr;
			sgs += slope_start_sg;
			sbs += slope_start_sb;
			sre += slope_end_sr;
			sge += slope_end_sg;
			sbe += slope_end_sb;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			buff_line += mem_pitch;
		}

		// draw the lower part (flat top triangle)

		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = r1+slope_r_v1v3*dmy1;
				gs = g1+slope_g_v1v3*dmy1;
				bs = b1+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = r2+slope_r_v2v3*delta_y;
				ge = g2+slope_g_v2v3*delta_y;
				be = b2+slope_b_v2v3*delta_y;
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Separate Specular Part
				srs = sr1+slope_sr_v1v3*dmy1;
				sgs = sg1+slope_sg_v1v3*dmy1;
				sbs = sb1+slope_sb_v1v3*dmy1;
				sre = sr2+slope_sr_v2v3*delta_y;
				sge = sg2+slope_sg_v2v3*delta_y;
				sbe = sb2+slope_sb_v2v3*delta_y;
				slope_start_sr = slope_sr_v1v3;
				slope_start_sg = slope_sg_v1v3;
				slope_start_sb = slope_sb_v1v3;
				slope_end_sr = slope_sr_v2v3;
				slope_end_sg = slope_sg_v2v3;
				slope_end_sb = slope_sb_v2v3;

				us = _u1 + slope_u_v1v3*dmy1;
				vs = _v1 + slope_v_v1v3*dmy1;
				ue = _u2 + slope_u_v2v3*delta_y;
				ve = _v2 + slope_v_v2v3*delta_y;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Separate Specular Part
				srs = sr2+slope_sr_v2v3*delta_y;
				sgs = sg2+slope_sg_v2v3*delta_y;
				sbs = sb2+slope_sb_v2v3*delta_y;
				sre = sr1+slope_sr_v1v3*dmy1;
				sge = sg1+slope_sg_v1v3*dmy1;
				sbe = sb1+slope_sb_v1v3*dmy1;
				slope_start_sr = slope_sr_v2v3;
				slope_start_sg = slope_sg_v2v3;
				slope_start_sb = slope_sb_v2v3;
				slope_end_sr = slope_sr_v1v3;
				slope_end_sg = slope_sg_v1v3;
				slope_end_sb = slope_sb_v1v3;

				us = _u2 + slope_u_v2v3*delta_y;
				vs = _v2 + slope_v_v2v3*delta_y;
				ue = _u1 + slope_u_v1v3*dmy1;
				ve = _v1 + slope_v_v1v3*dmy1;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;
			}

			ys = min_clip_y;
		}
		else
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = r1+slope_r_v1v3*dy21;
				gs = g1+slope_g_v1v3*dy21;
				bs = b1+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = r2;
				ge = g2;
				be = b2;
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Separate Specular Part
				srs = sr1+slope_sr_v1v3*dy21;
				sgs = sg1+slope_sg_v1v3*dy21;
				sbs = sb1+slope_sb_v1v3*dy21;
				sre = sr2;
				sge = sg2;
				sbe = sb2;
				slope_start_sr = slope_sr_v1v3;
				slope_start_sg = slope_sg_v1v3;
				slope_start_sb = slope_sb_v1v3;
				slope_end_sr = slope_sr_v2v3;
				slope_end_sg = slope_sg_v2v3;
				slope_end_sb = slope_sb_v2v3;
				
				us = _u1+slope_u_v1v3*dy21;
				vs = _v1+slope_v_v1v3*dy21;
				ue = _u2;
				ve = _v2;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = r2;
				gs = g2;
				bs = b2;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = r1+slope_r_v1v3*dy21;
				ge = g1+slope_g_v1v3*dy21;
				be = b1+slope_b_v1v3*dy21;
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Separate Specular Part
				srs = sr2;
				sgs = sg2;
				sbs = sb2;
				sre = sr1+slope_sr_v1v3*dy21;
				sge = sg1+slope_sg_v1v3*dy21;
				sbe = sb1+slope_sb_v1v3*dy21;
				slope_start_sr = slope_sr_v2v3;
				slope_start_sg = slope_sg_v2v3;
				slope_start_sb = slope_sb_v2v3;
				slope_end_sr = slope_sr_v1v3;
				slope_end_sg = slope_sg_v1v3;
				slope_end_sb = slope_sb_v1v3;
				
				us = _u2;
				vs = _v2;
				ue = _u1+slope_u_v1v3*dy21;
				ve = _v1+slope_v_v1v3*dy21;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			// Separate Specular Part
			sr=srs + NM3D_FIXP16_ROUND_UP;
			sg=sgs + NM3D_FIXP16_ROUND_UP;
			sb=sbs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Separate Specular Part
				dsr_x = (sre-srs)/dx;
				dsg_x = (sge-sgs)/dx;
				dsb_x = (sbe-sbs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Separate Specular Part
				dsr_x = (sre-srs);
				dsg_x = (sge-sgs);
				dsb_x = (sbe-sbs);

				du_x = (ue-us);
				dv_x = (ve-vs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Separate Specular Part
				sr += dsr_x*delta_x;
				sg += dsg_x*delta_x;
				sb += dsb_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;
				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SS_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Separate Specular Part
				sr += dsr_x;
				sg += dsg_x;
				sb += dsb_x;

				u += du_x;
				v += dv_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Separate Specular Part
			srs += slope_start_sr;
			sgs += slope_start_sg;
			sbs += slope_start_sb;
			sre += slope_end_sr;
			sge += slope_end_sg;
			sbe += slope_end_sb;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			buff_line += mem_pitch;
		}
	}
}

void CRender::TextureGouraudShadingSSZ(float poly[3][3], unsigned short *buff)
{
#define TEXTURE_GOURAUD_SS_Z_GET_POINT_VALUES	\
	do{ \
		x1=(int)(poly[v1][0]+0.0); y1=(int)(poly[v1][1]+0.0); z1=(int)(poly[v1][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x2=(int)(poly[v2][0]+0.0); y2=(int)(poly[v2][1]+0.0); z2=(int)(poly[v2][2]*(1<<NM3D_FIXP24_SHIFT)); \
		x3=(int)(poly[v3][0]+0.0); y3=(int)(poly[v3][1]+0.0); z3=(int)(poly[v3][2]*(1<<NM3D_FIXP24_SHIFT)); \
	}while(0)

/*
#if (NM3D_LIGHT_SEPERATE_SPECULAR)
	#define TEXTURE_GOURAUD_ADD_SPECULAR_PART	\
		r += m_rgbSpecular[]
*/

#ifdef NM3D_PIXELFORMAT_565 
	#define TEXTURE_GOURAUD_SS_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			rr += (sr<<5); \
			gg += (sg<<6); \
			bb += (sb<<5); \
			rr = rr>>(NM3D_FIXP16_SHIFT+5); rr = MIN(rr,255); \
			gg = gg>>(NM3D_FIXP16_SHIFT+6); gg = MIN(gg,255); \
			bb = bb>>(NM3D_FIXP16_SHIFT+5); bb = MIN(bb,255); \
			buff_line[x] = CRGB(rr,gg,bb).GetRGB(); \
		}
#else 
	#define TEXTURE_GOURAUD_SS_Z_SET_PIXEL	\
		if(zbuff_line[x]>=z) \
		{ \
			zbuff_line[x] = z; \
			rr += (sr<<5); \
			gg += (sg<<5); \
			bb += (sb<<5); \
			rr = rr>>(NM3D_FIXP16_SHIFT+5); rr = MIN(rr,255); \
			gg = gg>>(NM3D_FIXP16_SHIFT+5); gg = MIN(gg,255); \
			bb = bb>>(NM3D_FIXP16_SHIFT+5); bb = MIN(bb,255); \
			buff_line[x] = CRGB(rr,gg,bb).GetRGB(); \
		}
#endif


	//Z Buffer Variables
	int* zbuff=m_zbuffer.GetBuffer();
	int* zbuff_line=zbuff;
	int zmem_pitch = NM3D_WINDOW_WIDTH;
	int z1,z2,z3;
	int slope_start_z,slope_end_z;
	int z,zs,ze;
	int dz_x;
	
	int x1,y1,x2,y2,x3,y3; int delta_x,delta_y;
	int v1=0, v2=1, v3=2;

	int slope_start,slope_end;
	int r1,g1,b1,r2,g2,b2,r3,g3,b3;
	int slope_start_r,slope_start_g,slope_start_b;
	int slope_end_r,slope_end_g,slope_end_b;
	int x,y;int xstart,xend;
	int xs,xe;int ys,ye; //x start, x end, y start, y end
//	int dx_start=0,dx_end=0;
//	int dr_start=0,dg_start=0,db_start=0;
//	int dr_end=0,dg_end=0,db_end=0;
	int rs,re,gs,ge,bs,be;
	int dr_x,dg_x,db_x;
	int r,g,b;
	int dx,dy;

	//--start the following variables used by general triangle
	int dy21;
	int dy31;
	int dy32;
	
	int slope_v1v2;
	int slope_v1v3;
	int slope_v2v3;
	
	int slope_r_v1v2;
	int slope_g_v1v2;
	int slope_b_v1v2;
	int slope_r_v1v3;
	int slope_g_v1v3;
	int slope_b_v1v3;
	int slope_r_v2v3;
	int slope_g_v2v3;
	int slope_b_v2v3;
	
	// Separate Specular Part
	int slope_sr_v1v2;
	int slope_sg_v1v2;
	int slope_sb_v1v2;
	int slope_sr_v1v3;
	int slope_sg_v1v3;
	int slope_sb_v1v3;
	int slope_sr_v2v3;
	int slope_sg_v2v3;
	int slope_sb_v2v3;
	
	int slope_u_v1v2;
	int slope_v_v1v2;
	int slope_u_v1v3;
	int slope_v_v1v3;
	int slope_u_v2v3;
	int slope_v_v2v3;
	int dmy1;

	int slope_z_v1v2;
	int slope_z_v1v3;
	int slope_z_v2v3;

	//--end the above variables used by general triangle

	// Separate Specular Part
	int slope_start_sr,slope_start_sg,slope_start_sb;
	int slope_end_sr,slope_end_sg,slope_end_sb;
	int sr1,sg1,sb1,sr2,sg2,sb2,sr3,sg3,sb3;
//	int dsr_start=0,dsg_start=0,dsb_start=0;
//	int dsr_end=0,dsg_end=0,dsb_end=0;
	int srs,sre,sgs,sge,sbs,sbe;
	int dsr_x,dsg_x,dsb_x;
	int sr,sg,sb;

	// U,V
	int _u1,_u2,_u3,_v1,_v2,_v3;
	int slope_start_u,slope_start_v;
	int slope_end_u,slope_end_v;
//	int du_start=0,dv_start=0;
//	int du_end=0,dv_end=0;
	int du_x,dv_x;
	int us,ue,vs,ve;
	int u,v;
	unsigned int rr,gg,bb;
	CRGB* texture = (CRGB*)(m_pMaterial->m_imgTexture.GetData());
	unsigned char texture_width_log = Log2(m_pMaterial->m_imgTexture.GetWidth());
	unsigned short *buff_line = buff;
	int mem_pitch = m_nMemPitch>>1;

	for(int _i=0; _i<3; _i++)
	{
		poly[_i][0] = (int)(poly[_i][0]+0.0);
		poly[_i][1] = (int)(poly[_i][1]+0.0);
	}
	
	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||
		
		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||
		
		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||
		
		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
			return;

	if(poly[v1][1] > poly[v2][1])
		SWAP(v1,v2);
	if(poly[v1][1] > poly[v3][1])
		SWAP(v1,v3);
	
	// degenerate triangle
	TEXTURE_GOURAUD_SS_Z_GET_POINT_VALUES;
	if ( ((x1 == x2) && (x2 == x3)) || ((y1 ==  y2) && (y2 == y3)))
			return;

	if(!texture)
	{
		GouraudShading(poly,buff);
		return;
	}
	

	if(y1==y2 || y1==y3) // flat top
	{
		// swap to make sure pt[v1] is left most
		if(FCMP(poly[v1][1], poly[v2][1]))
		{
			if(poly[v1][0] > poly[v2][0])
			{
				SWAP(v1,v2);
				SWAP(x1,x2);
				SWAP(y1,y2);
				SWAP(z1,z2);
			}
		}
		else
		{
			if(poly[v1][0] > poly[v3][0])
			{
				SWAP(v1,v3);
				SWAP(x1,x3);
				SWAP(y1,y3);
				SWAP(z1,z3);
			}
		}
		
		// swap to make sure the flat top triangle has pt[v1] is top left, pt[v3] top right, and pt[v2] at bottom
		if(poly[v3][1] > poly[v2][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}

		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		dy = (y2-y1);//1.0f/(y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x2-x3)<<NM3D_FIXP16_SHIFT)/dy;
		m_rgba[v1].GetRGBValuec(r1,g1,b1); //r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); //r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); //r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r2-r3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g2-g3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b2-b3)<<NM3D_FIXP16_SHIFT)/dy;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1);// sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2);// sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3);// sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;
		slope_start_sr = ((sr2-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sg = ((sg2-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sb = ((sb2-sb1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sr = ((sr2-sr3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sg = ((sg2-sg3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sb = ((sb2-sb3)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u2-_u3)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v2-_v3)<<NM3D_FIXP16_SHIFT)/dy;

		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z2-z3)/dy;

		// 1/4 of clip: y start
		if (y1 <= min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x3<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r3<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g3<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b3<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT) + slope_start_sr*delta_y;
			sgs = (sg1<<NM3D_FIXP16_SHIFT) + slope_start_sg*delta_y;
			sbs = (sb1<<NM3D_FIXP16_SHIFT) + slope_start_sb*delta_y;
			sre = (sr3<<NM3D_FIXP16_SHIFT) + slope_end_sr*delta_y;
			sge = (sg3<<NM3D_FIXP16_SHIFT) + slope_end_sg*delta_y;
			sbe = (sb3<<NM3D_FIXP16_SHIFT) + slope_end_sb*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u3<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v3<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z3 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x3<<NM3D_FIXP16_SHIFT);
			re = (r3<<NM3D_FIXP16_SHIFT);
			ge = (g3<<NM3D_FIXP16_SHIFT);
			be = (b3<<NM3D_FIXP16_SHIFT);

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT);
			sgs = (sg1<<NM3D_FIXP16_SHIFT);
			sbs = (sb1<<NM3D_FIXP16_SHIFT);
			sre = (sr3<<NM3D_FIXP16_SHIFT);
			sge = (sg3<<NM3D_FIXP16_SHIFT);
			sbe = (sb3<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u3<<NM3D_FIXP16_SHIFT);
			ve = (_v3<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z3;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip: y end
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;

		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			// clip version		
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Separate Specular Part
					sr += dsr_x*delta_x;
					sg += dsg_x*delta_x;
					sb += dsb_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
				
					TEXTURE_GOURAUD_SS_Z_SET_PIXEL
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else // no x clip
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = ((xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);
				xend = ((xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT);

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;
				
				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_Z_SET_PIXEL
						
					r += dr_x;
					g += dg_x;
					b += db_x;
					
					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;
					
					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;
				
				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;
				
				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;
				
				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else if(y2 == y3) // flat bottom
	{
		// swap to make sure that pt[v1] is on top, pt[v2] bottom left, and pt[v3] bottom right
		if(poly[v2][0] > poly[v3][0])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}
		
		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		dy = (y2-y1);
		slope_start = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1);// r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2);// r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3);// r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;
		slope_start_r = ((r2-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_g = ((g2-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_b = ((b2-b1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_r = ((r3-r1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_g = ((g3-g1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_b = ((b3-b1)<<NM3D_FIXP16_SHIFT)/dy;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1);// sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2);// sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3);// sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;
		slope_start_sr = ((sr2-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sg = ((sg2-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_sb = ((sb2-sb1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sr = ((sr3-sr1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sg = ((sg3-sg1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_sb = ((sb3-sb1)<<NM3D_FIXP16_SHIFT)/dy;

		_u1 = (int)(m_fUCoords[v1]);
		_u2 = (int)(m_fUCoords[v2]);
		_u3 = (int)(m_fUCoords[v3]);
		_v1 = (int)(m_fVCoords[v1]);
		_v2 = (int)(m_fVCoords[v2]);
		_v3 = (int)(m_fVCoords[v3]);
		slope_start_u = ((_u2-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_start_v = ((_v2-_v1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_u = ((_u3-_u1)<<NM3D_FIXP16_SHIFT)/dy;
		slope_end_v = ((_v3-_v1)<<NM3D_FIXP16_SHIFT)/dy;

		// Z Buffer
		slope_start_z = (z2-z1)/dy;
		slope_end_z = (z3-z1)/dy;
		
		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = (r1<<NM3D_FIXP16_SHIFT) + slope_start_r*delta_y;
			gs = (g1<<NM3D_FIXP16_SHIFT) + slope_start_g*delta_y;
			bs = (b1<<NM3D_FIXP16_SHIFT) + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = (r1<<NM3D_FIXP16_SHIFT) + slope_end_r*delta_y;
			ge = (g1<<NM3D_FIXP16_SHIFT) + slope_end_g*delta_y;
			be = (b1<<NM3D_FIXP16_SHIFT) + slope_end_b*delta_y;

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT) + slope_start_sr*delta_y;
			sgs = (sg1<<NM3D_FIXP16_SHIFT) + slope_start_sg*delta_y;
			sbs = (sb1<<NM3D_FIXP16_SHIFT) + slope_start_sb*delta_y;
			sre = (sr1<<NM3D_FIXP16_SHIFT) + slope_end_sr*delta_y;
			sge = (sg1<<NM3D_FIXP16_SHIFT) + slope_end_sg*delta_y;
			sbe = (sb1<<NM3D_FIXP16_SHIFT) + slope_end_sb*delta_y;

			us = (_u1<<NM3D_FIXP16_SHIFT) + slope_start_u*delta_y;
			vs = (_v1<<NM3D_FIXP16_SHIFT) + slope_start_v*delta_y;
			ue = (_u1<<NM3D_FIXP16_SHIFT) + slope_end_u*delta_y;
			ve = (_v1<<NM3D_FIXP16_SHIFT) + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = (r1<<NM3D_FIXP16_SHIFT);
			gs = (g1<<NM3D_FIXP16_SHIFT);
			bs = (b1<<NM3D_FIXP16_SHIFT);
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = (r1<<NM3D_FIXP16_SHIFT);
			ge = (g1<<NM3D_FIXP16_SHIFT);
			be = (b1<<NM3D_FIXP16_SHIFT);

			// Separate Specular Part
			srs = (sr1<<NM3D_FIXP16_SHIFT);
			sgs = (sg1<<NM3D_FIXP16_SHIFT);
			sbs = (sb1<<NM3D_FIXP16_SHIFT);
			sre = (sr1<<NM3D_FIXP16_SHIFT);
			sge = (sg1<<NM3D_FIXP16_SHIFT);
			sbe = (sb1<<NM3D_FIXP16_SHIFT);
			
			us = (_u1<<NM3D_FIXP16_SHIFT);
			vs = (_v1<<NM3D_FIXP16_SHIFT);
			ue = (_u1<<NM3D_FIXP16_SHIFT);
			ve = (_v1<<NM3D_FIXP16_SHIFT);

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		if ((x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x) ||
			(x3 < min_clip_x) || (x3 > max_clip_x))
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;

				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				// 3/4 of clip: x start
				if(xstart < min_clip_x)
				{
					delta_x = (min_clip_x - xstart);
					r += dr_x*delta_x;
					g += dg_x*delta_x;
					b += db_x*delta_x;

					// Separate Specular Part
					sr += dsr_x*delta_x;
					sg += dsg_x*delta_x;
					sb += dsb_x*delta_x;

					u += du_x*delta_x;
					v += dv_x*delta_x;

					// Z Buffer
					z += dz_x*delta_x;

					xstart = min_clip_x;
				}
				
				// 4/4 of clip: x end
				if(xend > max_clip_x)
					xend = max_clip_x;
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_Z_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
		else
		{
			buff_line = buff + ys*mem_pitch;
			zbuff_line = zbuff + ys*zmem_pitch;

			for(y=ys; y<ye; y++)
			{
				xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
				xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;

				r=rs + NM3D_FIXP16_ROUND_UP;
				g=gs + NM3D_FIXP16_ROUND_UP;
				b=bs + NM3D_FIXP16_ROUND_UP;
				
				// Separate Specular Part
				sr=srs + NM3D_FIXP16_ROUND_UP;
				sg=sgs + NM3D_FIXP16_ROUND_UP;
				sb=sbs + NM3D_FIXP16_ROUND_UP;

				u=us + NM3D_FIXP16_ROUND_UP;
				v=vs + NM3D_FIXP16_ROUND_UP;

				// Z Buffer
				z=zs;
				
				if((dx = (xend-xstart)) > 0)
				{
					dr_x = (re-rs)/dx;
					dg_x = (ge-gs)/dx;
					db_x = (be-bs)/dx;

					// Separate Specular Part
					dsr_x = (sre-srs)/dx;
					dsg_x = (sge-sgs)/dx;
					dsb_x = (sbe-sbs)/dx;

					du_x = (ue-us)/dx;
					dv_x = (ve-vs)/dx;

					// Z Buffer
					dz_x = (ze-zs)/dx;
				}
				else
				{
					dr_x = (re-rs);
					dg_x = (ge-gs);
					db_x = (be-bs);

					// Separate Specular Part
					dsr_x = (sre-srs);
					dsg_x = (sge-sgs);
					dsb_x = (sbe-sbs);

					du_x = (ue-us);
					dv_x = (ve-vs);

					// Z Buffer
					dz_x = (ze-zs);
				}
				
				for(x=xstart; x<xend; x++)
				{
					texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
					rr *= r;
					gg *= g;
					bb *= b;
					
					TEXTURE_GOURAUD_SS_Z_SET_PIXEL	
					
					r += dr_x;
					g += dg_x;
					b += db_x;

					// Separate Specular Part
					sr += dsr_x;
					sg += dsg_x;
					sb += dsb_x;

					u += du_x;
					v += dv_x;

					// Z Buffer
					z += dz_x;
				}
				
				xs += slope_start;
				xe += slope_end;

				rs += slope_start_r;
				gs += slope_start_g;
				bs += slope_start_b;
				re += slope_end_r;
				ge += slope_end_g;
				be += slope_end_b;

				// Separate Specular Part
				srs += slope_start_sr;
				sgs += slope_start_sg;
				sbs += slope_start_sb;
				sre += slope_end_sr;
				sge += slope_end_sg;
				sbe += slope_end_sb;

				us += slope_start_u;
				vs += slope_start_v;
				ue += slope_end_u;
				ve += slope_end_v;

				// Z Buffer
				zs += slope_start_z;
				ze += slope_end_z;
				
				buff_line += mem_pitch;
				zbuff_line += zmem_pitch;
			}
		}
	}
	else // arbitrary triangle
	{
		if(poly[v2][1] > poly[v3][1])
		{
			SWAP(v2,v3);
			SWAP(x2,x3);
			SWAP(y2,y3);
			SWAP(z2,z3);
		}

		//TEXTURE_GOURAUD_SS_GET_POINT_VALUES;
		
		m_rgba[v1].GetRGBValuec(r1,g1,b1); r1<<=NM3D_FIXP16_SHIFT;g1<<=NM3D_FIXP16_SHIFT;b1<<=NM3D_FIXP16_SHIFT;
		m_rgba[v2].GetRGBValuec(r2,g2,b2); r2<<=NM3D_FIXP16_SHIFT;g2<<=NM3D_FIXP16_SHIFT;b2<<=NM3D_FIXP16_SHIFT;
		m_rgba[v3].GetRGBValuec(r3,g3,b3); r3<<=NM3D_FIXP16_SHIFT;g3<<=NM3D_FIXP16_SHIFT;b3<<=NM3D_FIXP16_SHIFT;

		// Separate Specular Part
		m_rgbaSpecular[v1].GetRGBValuec(sr1,sg1,sb1); sr1<<=NM3D_FIXP16_SHIFT;sg1<<=NM3D_FIXP16_SHIFT;sb1<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v2].GetRGBValuec(sr2,sg2,sb2); sr2<<=NM3D_FIXP16_SHIFT;sg2<<=NM3D_FIXP16_SHIFT;sb2<<=NM3D_FIXP16_SHIFT;
		m_rgbaSpecular[v3].GetRGBValuec(sr3,sg3,sb3); sr3<<=NM3D_FIXP16_SHIFT;sg3<<=NM3D_FIXP16_SHIFT;sb3<<=NM3D_FIXP16_SHIFT;

		_u1 = (((int)(m_fUCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_u2 = (((int)(m_fUCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_u3 = (((int)(m_fUCoords[v3]))<<NM3D_FIXP16_SHIFT);
		_v1 = (((int)(m_fVCoords[v1]))<<NM3D_FIXP16_SHIFT);
		_v2 = (((int)(m_fVCoords[v2]))<<NM3D_FIXP16_SHIFT);
		_v3 = (((int)(m_fVCoords[v3]))<<NM3D_FIXP16_SHIFT);

		dy21 = y2 - y1;
		dy31 = y3 - y1;
		dy32 = y3 - y2;
		
		slope_v1v2 = ((x2-x1)<<NM3D_FIXP16_SHIFT)/dy21;
		slope_v1v3 = ((x3-x1)<<NM3D_FIXP16_SHIFT)/dy31;
		slope_v2v3 = ((x3-x2)<<NM3D_FIXP16_SHIFT)/dy32;
		
		slope_r_v1v2 = (r2-r1)/dy21;
		slope_g_v1v2 = (g2-g1)/dy21;
		slope_b_v1v2 = (b2-b1)/dy21;
		slope_r_v1v3 = (r3-r1)/dy31;
		slope_g_v1v3 = (g3-g1)/dy31;
		slope_b_v1v3 = (b3-b1)/dy31;
		slope_r_v2v3 = (r3-r2)/dy32;
		slope_g_v2v3 = (g3-g2)/dy32;
		slope_b_v2v3 = (b3-b2)/dy32;

		// Separate Specular Part
		slope_sr_v1v2 = (sr2-sr1)/dy21;
		slope_sg_v1v2 = (sg2-sg1)/dy21;
		slope_sb_v1v2 = (sb2-sb1)/dy21;
		slope_sr_v1v3 = (sr3-sr1)/dy31;
		slope_sg_v1v3 = (sg3-sg1)/dy31;
		slope_sb_v1v3 = (sb3-sb1)/dy31;
		slope_sr_v2v3 = (sr3-sr2)/dy32;
		slope_sg_v2v3 = (sg3-sg2)/dy32;
		slope_sb_v2v3 = (sb3-sb2)/dy32;

		slope_u_v1v2 = (_u2-_u1)/dy21;
		slope_v_v1v2 = (_v2-_v1)/dy21;
		slope_u_v1v3 = (_u3-_u1)/dy31;
		slope_v_v1v3 = (_v3-_v1)/dy31;
		slope_u_v2v3 = (_u3-_u2)/dy32;
		slope_v_v2v3 = (_v3-_v2)/dy32;

		// Z Buffer
		slope_z_v1v2 = (z2-z1)/dy21;
		slope_z_v1v3 = (z3-z1)/dy31;
		slope_z_v2v3 = (z3-z2)/dy32;

		if(slope_v1v2 < slope_v1v3)
		{
			slope_start = slope_v1v2;
			slope_start_r = slope_r_v1v2;
			slope_start_g = slope_g_v1v2;
			slope_start_b = slope_b_v1v2;
			slope_end = slope_v1v3;
			slope_end_r = slope_r_v1v3;
			slope_end_g = slope_g_v1v3;
			slope_end_b = slope_b_v1v3;

			// Separate Specular Part
			slope_start_sr = slope_sr_v1v2;
			slope_start_sg = slope_sg_v1v2;
			slope_start_sb = slope_sb_v1v2;
			slope_end_sr = slope_sr_v1v3;
			slope_end_sg = slope_sg_v1v3;
			slope_end_sb = slope_sb_v1v3;

			slope_start_u = slope_u_v1v2;
			slope_start_v = slope_v_v1v2;
			slope_end_u = slope_u_v1v3;
			slope_end_v = slope_v_v1v3;

			// Z Buffer
			slope_start_z = slope_z_v1v2;
			slope_end_z = slope_z_v1v3;
		}
		else
		{
			slope_start = slope_v1v3;
			slope_start_r = slope_r_v1v3;
			slope_start_g = slope_g_v1v3;
			slope_start_b = slope_b_v1v3;
			slope_end = slope_v1v2;
			slope_end_r = slope_r_v1v2;
			slope_end_g = slope_g_v1v2;
			slope_end_b = slope_b_v1v2;

			// Separate Specular Part
			slope_start_sr = slope_sr_v1v3;
			slope_start_sg = slope_sg_v1v3;
			slope_start_sb = slope_sb_v1v3;
			slope_end_sr = slope_sr_v1v2;
			slope_end_sg = slope_sg_v1v2;
			slope_end_sb = slope_sb_v1v2;

			slope_start_u = slope_u_v1v3;
			slope_start_v = slope_v_v1v3;
			slope_end_u = slope_u_v1v2;
			slope_end_v = slope_v_v1v2;

			// Z Buffer
			slope_start_z = slope_z_v1v3;
			slope_end_z = slope_z_v1v2;
		}

		// 1/4 of clip
		if (y1 < min_clip_y)
		{
			delta_y = (min_clip_y - y1);

			xs = (x1<<NM3D_FIXP16_SHIFT) + slope_start*delta_y;
			rs = r1 + slope_start_r*delta_y;
			gs = g1 + slope_start_g*delta_y;
			bs = b1 + slope_start_b*delta_y;
			xe = (x1<<NM3D_FIXP16_SHIFT) + slope_end*delta_y;
			re = r1 + slope_end_r*delta_y;
			ge = g1 + slope_end_g*delta_y;
			be = b1 + slope_end_b*delta_y;

			// Separate Specular Part
			srs = sr1 + slope_start_sr*delta_y;
			sgs = sg1 + slope_start_sg*delta_y;
			sbs = sb1 + slope_start_sb*delta_y;
			sre = sr1 + slope_end_sr*delta_y;
			sge = sg1 + slope_end_sg*delta_y;
			sbe = sb1 + slope_end_sb*delta_y;

			us = _u1 + slope_start_u*delta_y;
			vs = _v1 + slope_start_v*delta_y;
			ue = _u1 + slope_end_u*delta_y;
			ve = _v1 + slope_end_v*delta_y;

			// Z Buffer
			zs = z1 + slope_start_z*delta_y;
			ze = z1 + slope_end_z*delta_y;

			ys = min_clip_y;
		}
		else
		{
			xs = (x1<<NM3D_FIXP16_SHIFT);
			rs = r1;
			gs = g1;
			bs = b1;
			xe = (x1<<NM3D_FIXP16_SHIFT);
			re = r1;
			ge = g1;
			be = b1;

			// Separate Specular Part
			srs = sr1;
			sgs = sg1;
			sbs = sb1;
			sre = sr1;
			sge = sg1;
			sbe = sb1;
			
			us = _u1;
			vs = _v1;
			ue = _u1;
			ve = _v1;

			// Z Buffer
			zs = z1;
			ze = z1;

			ys = y1; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y2) > max_clip_y)
			ye = max_clip_y;
		
		// draw the upper part (flat bottom triangle)
		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<=ye-1; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			// Separate Specular Part
			sr=srs + NM3D_FIXP16_ROUND_UP;
			sg=sgs + NM3D_FIXP16_ROUND_UP;
			sb=sbs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Separate Specular Part
				dsr_x = (sre-srs)/dx;
				dsg_x = (sge-sgs)/dx;
				dsb_x = (sbe-sbs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Separate Specular Part
				dsr_x = (sre-srs);
				dsg_x = (sge-sgs);
				dsb_x = (sbe-sbs);

				du_x = (ue-us);
				dv_x = (ve-vs);

				// Z Buffer
				dz_x = (ze-zs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Separate Specular Part
				sr += dsr_x*delta_x;
				sg += dsg_x*delta_x;
				sb += dsb_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;

			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SS_Z_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Separate Specular Part
				sr += dsr_x;
				sg += dsg_x;
				sb += dsb_x;

				u += du_x;
				v += dv_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Separate Specular Part
			srs += slope_start_sr;
			sgs += slope_start_sg;
			sbs += slope_start_sb;
			sre += slope_end_sr;
			sge += slope_end_sg;
			sbe += slope_end_sb;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;

			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}

		// draw the lower part (flat top triangle)

		// 1/4 of clip
		if (y2 < min_clip_y)
		{
			delta_y = (min_clip_y - y2);
			dmy1 = (min_clip_y-y1);
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				rs = r1+slope_r_v1v3*dmy1;
				gs = g1+slope_g_v1v3*dmy1;
				bs = b1+slope_b_v1v3*dmy1;
				xe = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				re = r2+slope_r_v2v3*delta_y;
				ge = g2+slope_g_v2v3*delta_y;
				be = b2+slope_b_v2v3*delta_y;
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Separate Specular Part
				srs = sr1+slope_sr_v1v3*dmy1;
				sgs = sg1+slope_sg_v1v3*dmy1;
				sbs = sb1+slope_sb_v1v3*dmy1;
				sre = sr2+slope_sr_v2v3*delta_y;
				sge = sg2+slope_sg_v2v3*delta_y;
				sbe = sb2+slope_sb_v2v3*delta_y;
				slope_start_sr = slope_sr_v1v3;
				slope_start_sg = slope_sg_v1v3;
				slope_start_sb = slope_sb_v1v3;
				slope_end_sr = slope_sr_v2v3;
				slope_end_sg = slope_sg_v2v3;
				slope_end_sb = slope_sb_v2v3;

				us = _u1 + slope_u_v1v3*dmy1;
				vs = _v1 + slope_v_v1v3*dmy1;
				ue = _u2 + slope_u_v2v3*delta_y;
				ve = _v2 + slope_v_v2v3*delta_y;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dmy1;
				ze = z2+slope_z_v2v3*delta_y;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT)+slope_v2v3*delta_y;
				rs = (r2)+slope_r_v2v3*delta_y;
				gs = (g2)+slope_g_v2v3*delta_y;
				bs = (b2)+slope_b_v2v3*delta_y;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dmy1;
				re = (r1)+slope_r_v1v3*dmy1;
				ge = (g1)+slope_g_v1v3*dmy1;
				be = (b1)+slope_b_v1v3*dmy1;
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Separate Specular Part
				srs = sr2+slope_sr_v2v3*delta_y;
				sgs = sg2+slope_sg_v2v3*delta_y;
				sbs = sb2+slope_sb_v2v3*delta_y;
				sre = sr1+slope_sr_v1v3*dmy1;
				sge = sg1+slope_sg_v1v3*dmy1;
				sbe = sb1+slope_sb_v1v3*dmy1;
				slope_start_sr = slope_sr_v2v3;
				slope_start_sg = slope_sg_v2v3;
				slope_start_sb = slope_sb_v2v3;
				slope_end_sr = slope_sr_v1v3;
				slope_end_sg = slope_sg_v1v3;
				slope_end_sb = slope_sb_v1v3;

				us = _u2 + slope_u_v2v3*delta_y;
				vs = _v2 + slope_v_v2v3*delta_y;
				ue = _u1 + slope_u_v1v3*dmy1;
				ve = _v1 + slope_v_v1v3*dmy1;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;

				// Z Buffer
				zs = z2+slope_z_v2v3*delta_y;
				ze = z1+slope_z_v1v3*dmy1;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = min_clip_y;
		}
		else
		{
			if(slope_v2v3 < slope_v1v3) // 13 -> 23 
			{
				xs = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				rs = r1+slope_r_v1v3*dy21;
				gs = g1+slope_g_v1v3*dy21;
				bs = b1+slope_b_v1v3*dy21;
				xe = (x2<<NM3D_FIXP16_SHIFT);
				re = r2;
				ge = g2;
				be = b2;
				slope_start = slope_v1v3;
				slope_start_r = slope_r_v1v3;
				slope_start_g = slope_g_v1v3;
				slope_start_b = slope_b_v1v3;
				slope_end = slope_v2v3;
				slope_end_r = slope_r_v2v3;
				slope_end_g = slope_g_v2v3;
				slope_end_b = slope_b_v2v3;

				// Separate Specular Part
				srs = sr1+slope_sr_v1v3*dy21;
				sgs = sg1+slope_sg_v1v3*dy21;
				sbs = sb1+slope_sb_v1v3*dy21;
				sre = sr2;
				sge = sg2;
				sbe = sb2;
				slope_start_sr = slope_sr_v1v3;
				slope_start_sg = slope_sg_v1v3;
				slope_start_sb = slope_sb_v1v3;
				slope_end_sr = slope_sr_v2v3;
				slope_end_sg = slope_sg_v2v3;
				slope_end_sb = slope_sb_v2v3;
				
				us = _u1+slope_u_v1v3*dy21;
				vs = _v1+slope_v_v1v3*dy21;
				ue = _u2;
				ve = _v2;
				slope_start_u = slope_u_v1v3;
				slope_start_v = slope_v_v1v3;
				slope_end_u = slope_u_v2v3;
				slope_end_v = slope_v_v2v3;

				// Z Buffer
				zs = z1+slope_z_v1v3*dy21;
				ze = z2;
				slope_start_z = slope_z_v1v3;
				slope_end_z = slope_z_v2v3;
			}
			else // 23 -> 13 
			{
				xs = (x2<<NM3D_FIXP16_SHIFT);
				rs = r2;
				gs = g2;
				bs = b2;
				xe = (x1<<NM3D_FIXP16_SHIFT)+slope_v1v3*dy21;
				re = r1+slope_r_v1v3*dy21;
				ge = g1+slope_g_v1v3*dy21;
				be = b1+slope_b_v1v3*dy21;
				slope_start = slope_v2v3;
				slope_start_r = slope_r_v2v3;
				slope_start_g = slope_g_v2v3;
				slope_start_b = slope_b_v2v3;
				slope_end = slope_v1v3;
				slope_end_r = slope_r_v1v3;
				slope_end_g = slope_g_v1v3;
				slope_end_b = slope_b_v1v3;

				// Separate Specular Part
				srs = sr2;
				sgs = sg2;
				sbs = sb2;
				sre = sr1+slope_sr_v1v3*dy21;
				sge = sg1+slope_sg_v1v3*dy21;
				sbe = sb1+slope_sb_v1v3*dy21;
				slope_start_sr = slope_sr_v2v3;
				slope_start_sg = slope_sg_v2v3;
				slope_start_sb = slope_sb_v2v3;
				slope_end_sr = slope_sr_v1v3;
				slope_end_sg = slope_sg_v1v3;
				slope_end_sb = slope_sb_v1v3;
				
				us = _u2;
				vs = _v2;
				ue = _u1+slope_u_v1v3*dy21;
				ve = _v1+slope_v_v1v3*dy21;
				slope_start_u = slope_u_v2v3;
				slope_start_v = slope_v_v2v3;
				slope_end_u = slope_u_v1v3;
				slope_end_v = slope_v_v1v3;

				// Z Buffer
				zs = z2;
				ze = z1+slope_z_v1v3*dy21;
				slope_start_z = slope_z_v2v3;
				slope_end_z = slope_z_v1v3;
			}

			ys = y2; // no clipping
		}
		
		// 2/4 of clip
		if ((ye = y3) > max_clip_y)
			ye = max_clip_y;

		buff_line = buff + ys*mem_pitch;
		zbuff_line = zbuff + ys*zmem_pitch;

		for(y=ys; y<ye; y++)
		{
			xstart = (xs + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			xend = (xe + NM3D_FIXP16_ROUND_UP)>>NM3D_FIXP16_SHIFT;
			
			r=rs + NM3D_FIXP16_ROUND_UP;
			g=gs + NM3D_FIXP16_ROUND_UP;
			b=bs + NM3D_FIXP16_ROUND_UP;
			
			// Separate Specular Part
			sr=srs + NM3D_FIXP16_ROUND_UP;
			sg=sgs + NM3D_FIXP16_ROUND_UP;
			sb=sbs + NM3D_FIXP16_ROUND_UP;
			
			u=us + NM3D_FIXP16_ROUND_UP;
			v=vs + NM3D_FIXP16_ROUND_UP;

			// Z Buffer
			z=zs;
			
			if((dx = (xend-xstart)) > 0)
			{
				dr_x = (re-rs)/dx;
				dg_x = (ge-gs)/dx;
				db_x = (be-bs)/dx;

				// Separate Specular Part
				dsr_x = (sre-srs)/dx;
				dsg_x = (sge-sgs)/dx;
				dsb_x = (sbe-sbs)/dx;

				du_x = (ue-us)/dx;
				dv_x = (ve-vs)/dx;

				// Z Buffer
				dz_x = (ze-zs)/dx;
			}
			else
			{
				dr_x = (re-rs);
				dg_x = (ge-gs);
				db_x = (be-bs);

				// Separate Specular Part
				dsr_x = (sre-srs);
				dsg_x = (sge-sgs);
				dsb_x = (sbe-sbs);

				du_x = (ue-us);
				dv_x = (ve-vs);

				// Z Buffer
				dz_x = (ze-zs);
			}

			// 3/4 of clip: x start
			if(xstart < min_clip_x)
			{
				delta_x = (min_clip_x - xstart);
				r += dr_x*delta_x;
				g += dg_x*delta_x;
				b += db_x*delta_x;

				// Separate Specular Part
				sr += dsr_x*delta_x;
				sg += dsg_x*delta_x;
				sb += dsb_x*delta_x;

				u += du_x*delta_x;
				v += dv_x*delta_x;

				// Z Buffer
				z += dz_x*delta_x;

				xstart = min_clip_x;
			}
			
			// 4/4 of clip: x end
			if(xend > max_clip_x)
				xend = max_clip_x;
			
			for(x=xstart; x<xend; x++)
			{
				texture[((v>>NM3D_FIXP16_SHIFT)<<texture_width_log)+(u>>NM3D_FIXP16_SHIFT)].GetRGBValue(rr,gg,bb);
				rr *= r;
				gg *= g;
				bb *= b;
				
				TEXTURE_GOURAUD_SS_Z_SET_PIXEL
				
				r += dr_x;
				g += dg_x;
				b += db_x;

				// Separate Specular Part
				sr += dsr_x;
				sg += dsg_x;
				sb += dsb_x;

				u += du_x;
				v += dv_x;

				// Z Buffer
				z += dz_x;
			}
			
			xs += slope_start;
			xe += slope_end;
			
			rs += slope_start_r;
			gs += slope_start_g;
			bs += slope_start_b;
			re += slope_end_r;
			ge += slope_end_g;
			be += slope_end_b;

			// Separate Specular Part
			srs += slope_start_sr;
			sgs += slope_start_sg;
			sbs += slope_start_sb;
			sre += slope_end_sr;
			sge += slope_end_sg;
			sbe += slope_end_sb;

			us += slope_start_u;
			vs += slope_start_v;
			ue += slope_end_u;
			ve += slope_end_v;

			// Z Buffer
			zs += slope_start_z;
			ze += slope_end_z;

			buff_line += mem_pitch;
			zbuff_line += zmem_pitch;
		}
	}
}
// Texture Gouraud Shading end
////////////////////////////////////////////////////////////////////////////


#if (1)
	void CRender::Draw_Triangle_2D3_16(float poly[3][3], unsigned short *buff)
	{
		// this function draws a flat shaded polygon with zbuffering
		int mem_pitch = m_nMemPitch;
		int v0=0,
			v1=1,
			v2=2,
			//temp=0,
			tri_type = NM3D_TRI_TYPE_NONE,
			irestart = NM3D_INTERP_LHS;
		
		int //dx,
			dy,dyl,dyr,      // general deltas
			xi,yi,              // the current interpolated x,y
			//		index_x,index_y,    // looping vars
			//		x,y,                // hold general x,y
			xstart,
			xend,
			ystart,
			yrestart,
			yend,
			xl,                 
			dxdyl,              
			xr,
			dxdyr;
		
		int x0,y0,    // cached vertices
			x1,y1,
			x2,y2;
		
		unsigned short *screen_ptr  = NULL,
//			*screen_line = NULL,
//			*textmap     = NULL,
			*dest_buffer = buff;
		
		unsigned short color;    // polygon color
		
#ifdef DEBUG_ON
		// track rendering stats
		debug_polys_rendered_per_frame++;
#endif
		
		// adjust memory pitch to words, divide by 2
		mem_pitch >>=1;
		
		// apply fill convention to coordinates
		poly[0][0] = (int)(poly[0][0]+0.5);
		poly[0][1] = (int)(poly[0][1]+0.5);
		
		poly[1][0] = (int)(poly[1][0]+0.5);
		poly[1][1] = (int)(poly[1][1]+0.5);
		
		poly[2][0] = (int)(poly[2][0]+0.5);
		poly[2][1] = (int)(poly[2][1]+0.5);
		
		
		// first trivial clipping rejection tests 
		if (((poly[0][1] < min_clip_y)  && 
			(poly[1][1] < min_clip_y)  &&
			(poly[2][1] < min_clip_y)) ||
			
			((poly[0][1] > max_clip_y)  && 
			(poly[1][1] > max_clip_y)  &&
			(poly[2][1] > max_clip_y)) ||
			
			((poly[0][0] < min_clip_x)  && 
			(poly[1][0] < min_clip_x)  &&
			(poly[2][0] < min_clip_x)) ||
			
			((poly[0][0] > max_clip_x)  && 
			(poly[1][0] > max_clip_x)  &&
			(poly[2][0] > max_clip_x)))
			return;
		
		
		// sort vertices
		if (poly[v1][1] < poly[v0][1]) 
		{SWAP(v0,v1/*,temp*/);} 
		
		if (poly[v2][1] < poly[v0][1]) 
		{SWAP(v0,v2/*,temp*/);}
		
		if (poly[v2][1] < poly[v1][1]) 
		{SWAP(v1,v2/*,temp*/);}
		
		// now test for trivial flat sided cases
		if (FCMP(poly[v0][1], poly[v1][1]) )
		{ 
			// set triangle type
			tri_type = NM3D_TRI_TYPE_FLAT_TOP;
			
			// sort vertices left to right
			if (poly[v1][0] < poly[v0][0]) 
			{SWAP(v0,v1/*,temp*/);}
			
		} // end if
		else
		{
			// now test for trivial flat sided cases
			if (FCMP(poly[v1][1], poly[v2][1]) )
			{ 
				// set triangle type
				tri_type = NM3D_TRI_TYPE_FLAT_BOTTOM;
				
				// sort vertices left to right
				if (poly[v2][0] < poly[v1][0]) 
				{SWAP(v1,v2/*,temp*/);}
				
			} // end if
			else
			{
				// must be a general triangle
				tri_type = NM3D_TRI_TYPE_GENERAL;
				
			} // end else
		}
		// extract vertices for processing, now that we have order
		x0 = (int)(poly[v0][0] + 0.0);
		y0 = (int)(poly[v0][1] + 0.0);

		x1 = (int)(poly[v1][0] + 0.0);
		y1 = (int)(poly[v1][1] + 0.0);

		x2 = (int)(poly[v2][0] + 0.0);
		y2 = (int)(poly[v2][1] + 0.0);

		// degenerate triangle
		if (((x0 == x1) && (x1 == x2)) || ((y0 == y1) && (y1 == y2)))
			return;

		// extract constant color
		color = m_rgba[0].GetRGB(); // 0x7f;//RGB555(31,0,0);//face->lit_color[0];

		// set interpolation restart value
		yrestart = y1;

		// what kind of triangle
		if (tri_type & NM3D_TRI_TYPE_FLAT_MASK)
		{

			if (tri_type == NM3D_TRI_TYPE_FLAT_TOP)
			{
				// compute all deltas
				dy = (y2 - y0);

				dxdyl = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dy;
				dxdyr = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dy;

				// test for y clipping
				if (y0 < min_clip_y)
				{
					// compute overclip
					dy = (min_clip_y - y0);

					// computer new LHS starting values
					xl = dxdyl * dy + (x0 << NM3D_FIXP16_SHIFT);

					// compute new RHS starting values
					xr = dxdyr * dy + (x1 << NM3D_FIXP16_SHIFT);

					// compute new starting y
					ystart = min_clip_y;

				} // end if
				else
				{
					// no clipping

					// set starting values
					xl = (x0 << NM3D_FIXP16_SHIFT);
					xr = (x1 << NM3D_FIXP16_SHIFT);

					// set starting y
					ystart = y0;

				} // end else

			} // end if flat top
			else
			{
				// must be flat bottom

				// compute all deltas
				dy = (y1 - y0);

				dxdyl = ((x1 - x0) << NM3D_FIXP16_SHIFT) / dy;
				dxdyr = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dy;

				// test for y clipping
				if (y0 < min_clip_y)
				{
					// compute overclip
					dy = (min_clip_y - y0);

					// computer new LHS starting values
					xl = dxdyl * dy + (x0 << NM3D_FIXP16_SHIFT);

					// compute new RHS starting values
					xr = dxdyr * dy + (x0 << NM3D_FIXP16_SHIFT);

					// compute new starting y
					ystart = min_clip_y;

				} // end if
				else
				{
					// no clipping

					// set starting values
					xl = (x0 << NM3D_FIXP16_SHIFT);
					xr = (x0 << NM3D_FIXP16_SHIFT);

					// set starting y
					ystart = y0;

				} // end else

			} // end else flat bottom

			// test for bottom clip, always
			if ((yend = y2) > max_clip_y)
				yend = max_clip_y;

			// test for horizontal clipping
			if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
				(x1 < min_clip_x) || (x1 > max_clip_x) ||
				(x2 < min_clip_x) || (x2 > max_clip_x))
			{
				// clip version

				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					//dx = (xend - xstart);

					///////////////////////////////////////////////////////////////////////

					// test for x clipping, LHS
					if (xstart < min_clip_x)
					{
						// compute x overlap
						//dx = min_clip_x - xstart;

						// reset vars
						xstart = min_clip_x;

					} // end if

					// test for x clipping RHS
					if (xend > max_clip_x)
						xend = max_clip_x;

					///////////////////////////////////////////////////////////////////////

					// draw span
					for (xi = xstart; xi < xend; xi++)
					{
						// write textel
						screen_ptr[xi] = color;
					} // end for xi
					// MemSet16(screen_ptr+xi, color, xend-xstart);

					// interpolate x along right and left edge
					xl += dxdyl;
					xr += dxdyr;

					// advance screen ptr
					screen_ptr += mem_pitch;

				} // end for y

			} // end if clip
			else
			{
				// non-clip version

				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					//dx = (xend - xstart);

					// draw span
					for (xi = xstart; xi < xend; xi++)
					{
						// write textel
						screen_ptr[xi] = color;
					} // end for xi

					// interpolate x,z along right and left edge
					xl += dxdyl;
					xr += dxdyr;

					// advance screen ptr
					screen_ptr += mem_pitch;

				} // end for y

			} // end if non-clipped

		} // end if
		else
		{
			if (tri_type==NM3D_TRI_TYPE_GENERAL)
			{
				
				// first test for bottom clip, always
				if ((yend = y2) > max_clip_y)
					yend = max_clip_y;
				
				// pre-test y clipping status
				if (y1 < min_clip_y)
				{
					// compute all deltas
					// LHS
					dyl = (y2 - y1);
					
					dxdyl = ((x2  - x1)  << NM3D_FIXP16_SHIFT)/dyl;
					
					// RHS
					dyr = (y2 - y0);	
					
					dxdyr = ((x2  - x0)  << NM3D_FIXP16_SHIFT)/dyr;
					
					// compute overclip
					dyr = (min_clip_y - y0);
					dyl = (min_clip_y - y1);
					
					// computer new LHS starting values
					xl = dxdyl*dyl + (x1  << NM3D_FIXP16_SHIFT);
					
					// compute new RHS starting values
					xr = dxdyr*dyr + (x0  << NM3D_FIXP16_SHIFT);
					
					// compute new starting y
					ystart = min_clip_y;
					
					// test if we need swap to keep rendering left to right
					if (dxdyr > dxdyl)
					{
						SWAP(dxdyl,dxdyr/*,temp*/);
						SWAP(xl,xr/*,temp*/);
						SWAP(x1,x2/*,temp*/);
						SWAP(y1,y2/*,temp*/);
						
						// set interpolation restart
						irestart = NM3D_INTERP_RHS;
						
					} // end if
					
				} // end if
				else
				{
					if (y0 < min_clip_y)
					{
						// compute all deltas
						// LHS
						dyl = (y1 - y0);
						
						dxdyl = ((x1  - x0)  << NM3D_FIXP16_SHIFT)/dyl;
						
						// RHS
						dyr = (y2 - y0);	
						
						dxdyr = ((x2  - x0)  << NM3D_FIXP16_SHIFT)/dyr;
						
						// compute overclip
						dy = (min_clip_y - y0);
						
						// computer new LHS starting values
						xl = dxdyl*dy + (x0  << NM3D_FIXP16_SHIFT);
						
						// compute new RHS starting values
						xr = dxdyr*dy + (x0  << NM3D_FIXP16_SHIFT);
						
						// compute new starting y
						ystart = min_clip_y;
						
						// test if we need swap to keep rendering left to right
						if (dxdyr < dxdyl)
						{
							SWAP(dxdyl,dxdyr/*,temp*/);
							SWAP(xl,xr/*,temp*/);
							SWAP(x1,x2/*,temp*/);
							SWAP(y1,y2/*,temp*/);
							
							// set interpolation restart
							irestart = NM3D_INTERP_RHS;
							
						} // end if
						
					} // end if
					else
					{
						// no initial y clipping
						
						// compute all deltas
						// LHS
						dyl = (y1 - y0);
						
						dxdyl = ((x1  - x0)  << NM3D_FIXP16_SHIFT)/dyl;
						
						// RHS
						dyr = (y2 - y0);	
						
						dxdyr = ((x2 - x0)   << NM3D_FIXP16_SHIFT)/dyr;
						
						// no clipping y
						
						// set starting values
						xl = (x0 << NM3D_FIXP16_SHIFT);
						xr = (x0 << NM3D_FIXP16_SHIFT);
						
						// set starting y
						ystart = y0;
						
						// test if we need swap to keep rendering left to right
						if (dxdyr < dxdyl)
						{
							SWAP(dxdyl,dxdyr/*,temp*/);
							SWAP(xl,xr/*,temp*/);
							SWAP(x1,x2/*,temp*/);
							SWAP(y1,y2/*,temp*/);
							
							// set interpolation restart
							irestart = NM3D_INTERP_RHS;
							
						} // end if
						
					} // end else
				}

				// test for horizontal clipping
				if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
					(x1 < min_clip_x) || (x1 > max_clip_x) ||
					(x2 < min_clip_x) || (x2 > max_clip_x))
				{
					// clip version
					// x clipping

					// point screen ptr to starting line
					screen_ptr = dest_buffer + (ystart * mem_pitch);

					for (yi = ystart; yi < yend; yi++)
					{
						// compute span endpoints
						xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
						xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

						//dx = (xend - xstart);

						///////////////////////////////////////////////////////////////////////

						// test for x clipping, LHS
						if (xstart < min_clip_x)
						{
							// compute x overlap
							//dx = min_clip_x - xstart;

							// set x to left clip edge
							xstart = min_clip_x;

						} // end if

						// test for x clipping RHS
						if (xend > max_clip_x)
							xend = max_clip_x;

						///////////////////////////////////////////////////////////////////////

						// draw span
						for (xi = xstart; xi < xend; xi++)
						{
							// write textel assume 5.6.5
							screen_ptr[xi] = color;

						} // end for xi

						// interpolate z,x along right and left edge
						xl += dxdyl;
						xr += dxdyr;

						// advance screen ptr
						screen_ptr += mem_pitch;

						// test for yi hitting second region, if so change interpolant
						if (yi == yrestart)
						{
							// test interpolation side change flag

							if (irestart == NM3D_INTERP_LHS)
							{
								// LHS
								dyl = (y2 - y1);

								dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;

								// set starting values
								xl = (x1 << NM3D_FIXP16_SHIFT);

								// interpolate down on LHS to even up
								xl += dxdyl;
							} // end if
							else
							{
								// RHS
								dyr = (y1 - y2);

								dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;

								// set starting values
								xr = (x2 << NM3D_FIXP16_SHIFT);

								// interpolate down on RHS to even up
								xr += dxdyr;

							} // end else

						} // end if

					} // end for y

				} // end if
				else
				{
					// no x clipping
					// point screen ptr to starting line
					screen_ptr = dest_buffer + (ystart * mem_pitch);

					for (yi = ystart; yi < yend; yi++)
					{
						// compute span endpoints
						xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
						xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

						//dx = (xend - xstart);

						// draw span
						for (xi = xstart; xi < xend; xi++)
						{
							// write textel assume 5.6.5
							screen_ptr[xi] = color;
						} // end for xi

						// interpolate x,z along right and left edge
						xl += dxdyl;
						xr += dxdyr;

						// advance screen ptr
						screen_ptr += mem_pitch;

						// test for yi hitting second region, if so change interpolant
						if (yi == yrestart)
						{
							// test interpolation side change flag

							if (irestart == NM3D_INTERP_LHS)
							{
								// LHS
								dyl = (y2 - y1);

								dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;

								// set starting values
								xl = (x1 << NM3D_FIXP16_SHIFT);

								// interpolate down on LHS to even up
								xl += dxdyl;
							} // end if
							else
							{
								// RHS
								dyr = (y1 - y2);

								dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;

								// set starting values
								xr = (x2 << NM3D_FIXP16_SHIFT);

								// interpolate down on RHS to even up
								xr += dxdyr;
							} // end else

						} // end if

					} // end for y

				} // end else

			} // end if
		}
	} // end Draw_Triangle_2D3_16
#endif


}

////////////////////////////////////////////////////////////////////////////////

void Nomad3D::CRender::Draw_Gouraud_Triangle2_16(float poly[3][3], UCHAR *_dest_buffer)
{
	// this function draws a gouraud shaded polygon, based on the affine texture mapper, instead
	// of interpolating the texture coordinates, we simply interpolate the (R,G,B) values across
	// the polygons, I simply needed at another interpolant, I have mapped u->red, v->green, w->blue
	int mem_pitch = m_nMemPitch;
	int v0 = 0,
		v1 = 1,
		v2 = 2,
		//temp = 0,
		tri_type = NM3D_TRI_TYPE_NONE,
		irestart = NM3D_INTERP_LHS;

	int dx, dy, dyl, dyr, // general deltas
						  //		u,v,w,
		du, dv, dw,
		xi, yi,		// the current interpolated x,y
		ui, vi, wi, // the current interpolated u,v
					//		index_x,index_y,    // looping vars
					//		x,y,                // hold general x,y
		xstart,
		xend,
		ystart,
		yrestart,
		yend,
		xl,
		dxdyl,
		xr,
		dxdyr,
		dudyl,
		ul,
		dvdyl,
		vl,
		dwdyl,
		wl,
		dudyr,
		ur,
		dvdyr,
		vr,
		dwdyr,
		wr;

	int x0, y0, tu0, tv0, tw0, // cached vertices
		x1, y1, tu1, tv1, tw1,
		x2, y2, tu2, tv2, tw2;

	int r_base0, g_base0, b_base0,
		r_base1, g_base1, b_base1,
		r_base2, g_base2, b_base2;

	USHORT *screen_ptr = NULL,
		   //*screen_line = NULL,
		   //*textmap = NULL,
		   *dest_buffer = (USHORT *)_dest_buffer;

#ifdef DEBUG_ON
	// track rendering stats
	debug_polys_rendered_per_frame++;
#endif

	// adjust memory pitch to words, divide by 2
	mem_pitch >>= 1;

	// apply fill convention to coordinates
	poly[0][0] = (int)(poly[0][0] + 0.0);
	poly[0][1] = (int)(poly[0][1] + 0.0);

	poly[1][0] = (int)(poly[1][0] + 0.0);
	poly[1][1] = (int)(poly[1][1] + 0.0);

	poly[2][0] = (int)(poly[2][0] + 0.0);
	poly[2][1] = (int)(poly[2][1] + 0.0);

	// first trivial clipping rejection tests
	if (((poly[0][1] < min_clip_y) &&
		 (poly[1][1] < min_clip_y) &&
		 (poly[2][1] < min_clip_y)) ||

		((poly[0][1] > max_clip_y) &&
		 (poly[1][1] > max_clip_y) &&
		 (poly[2][1] > max_clip_y)) ||

		((poly[0][0] < min_clip_x) &&
		 (poly[1][0] < min_clip_x) &&
		 (poly[2][0] < min_clip_x)) ||

		((poly[0][0] > max_clip_x) &&
		 (poly[1][0] > max_clip_x) &&
		 (poly[2][0] > max_clip_x)))
		return;

	// sort vertices
	if (poly[v1][1] < poly[v0][1])
	{
		SWAP(v0, v1 /*,temp*/);
	}

	if (poly[v2][1] < poly[v0][1])
	{
		SWAP(v0, v2 /*,temp*/);
	}

	if (poly[v2][1] < poly[v1][1])
	{
		SWAP(v1, v2 /*,temp*/);
	}

	// now test for trivial flat sided cases
	if (FCMP(poly[v0][1], poly[v1][1]))
	{
		// set triangle type
		tri_type = NM3D_TRI_TYPE_FLAT_TOP;

		// sort vertices left to right
		if (poly[v1][0] < poly[v0][0])
		{
			SWAP(v0, v1 /*,temp*/);
		}

	} // end if
	else
		// now test for trivial flat sided cases
		if (FCMP(poly[v1][1], poly[v2][1]))
		{
			// set triangle type
			tri_type = NM3D_TRI_TYPE_FLAT_BOTTOM;

			// sort vertices left to right
			if (poly[v2][0] < poly[v1][0])
			{
				SWAP(v1, v2 /*,temp*/);
			}

		} // end if
		else
		{
			// must be a general triangle
			tri_type = NM3D_TRI_TYPE_GENERAL;

		} // end else

	// assume 5.6.5 format -- sorry!
	// we can't afford a function call in the inner loops, so we must write
	// two hard coded versions, if we want support for both 5.6.5, and 5.5.5

	//		_RGB555FROM16BIT(face->lit_color[v0], &r_base0, &g_base0, &b_base0);
	m_rgba[v0].GetRGBValuec(r_base0, g_base0, b_base0);
	//		_RGB555FROM16BIT(face->lit_color[v1], &r_base1, &g_base1, &b_base1);
	m_rgba[v1].GetRGBValuec(r_base1, g_base1, b_base1);
	//		_RGB555FROM16BIT(face->lit_color[v2], &r_base2, &g_base2, &b_base2);
	m_rgba[v2].GetRGBValuec(r_base2, g_base2, b_base2);

	// extract vertices for processing, now that we have order
	x0 = (int)(poly[v0][0] + 0.0);
	y0 = (int)(poly[v0][1] + 0.0);

	tu0 = r_base0;
	tv0 = g_base0;
	tw0 = b_base0;

	x1 = (int)(poly[v1][0] + 0.0);
	y1 = (int)(poly[v1][1] + 0.0);

	tu1 = r_base1;
	tv1 = g_base1;
	tw1 = b_base1;

	x2 = (int)(poly[v2][0] + 0.0);
	y2 = (int)(poly[v2][1] + 0.0);

	tu2 = r_base2;
	tv2 = g_base2;
	tw2 = b_base2;

	// degenerate triangle
	if (((x0 == x1) && (x1 == x2)) || ((y0 == y1) && (y1 == y2)))
		return;

	// set interpolation restart value
	yrestart = y1;

	// what kind of triangle
	if (tri_type & NM3D_TRI_TYPE_FLAT_MASK)
	{

		if (tri_type == NM3D_TRI_TYPE_FLAT_TOP)
		{
			// compute all deltas
			dy = (y2 - y0);

			dxdyl = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dy;
			dudyl = ((tu2 - tu0) << NM3D_FIXP16_SHIFT) / dy;
			dvdyl = ((tv2 - tv0) << NM3D_FIXP16_SHIFT) / dy;
			dwdyl = ((tw2 - tw0) << NM3D_FIXP16_SHIFT) / dy;

			dxdyr = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dy;
			dudyr = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dy;
			dvdyr = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dy;
			dwdyr = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dy;

			// test for y clipping
			if (y0 < min_clip_y)
			{
				// compute overclip
				dy = (min_clip_y - y0);

				// computer new LHS starting values
				xl = dxdyl * dy + (x0 << NM3D_FIXP16_SHIFT);
				ul = dudyl * dy + (tu0 << NM3D_FIXP16_SHIFT);
				vl = dvdyl * dy + (tv0 << NM3D_FIXP16_SHIFT);
				wl = dwdyl * dy + (tw0 << NM3D_FIXP16_SHIFT);

				// compute new RHS starting values
				xr = dxdyr * dy + (x1 << NM3D_FIXP16_SHIFT);
				ur = dudyr * dy + (tu1 << NM3D_FIXP16_SHIFT);
				vr = dvdyr * dy + (tv1 << NM3D_FIXP16_SHIFT);
				wr = dwdyr * dy + (tw1 << NM3D_FIXP16_SHIFT);

				// compute new starting y
				ystart = min_clip_y;

			} // end if
			else
			{
				// no clipping

				// set starting values
				xl = (x0 << NM3D_FIXP16_SHIFT);
				xr = (x1 << NM3D_FIXP16_SHIFT);

				ul = (tu0 << NM3D_FIXP16_SHIFT);
				vl = (tv0 << NM3D_FIXP16_SHIFT);
				wl = (tw0 << NM3D_FIXP16_SHIFT);

				ur = (tu1 << NM3D_FIXP16_SHIFT);
				vr = (tv1 << NM3D_FIXP16_SHIFT);
				wr = (tw1 << NM3D_FIXP16_SHIFT);

				// set starting y
				ystart = y0;

			} // end else

		} // end if flat top
		else
		{
			// must be flat bottom

			// compute all deltas
			dy = (y1 - y0);

			dxdyl = ((x1 - x0) << NM3D_FIXP16_SHIFT) / dy;
			dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT) / dy;
			dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT) / dy;
			dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT) / dy;

			dxdyr = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dy;
			dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT) / dy;
			dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT) / dy;
			dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT) / dy;

			// test for y clipping
			if (y0 < min_clip_y)
			{
				// compute overclip
				dy = (min_clip_y - y0);

				// computer new LHS starting values
				xl = dxdyl * dy + (x0 << NM3D_FIXP16_SHIFT);
				ul = dudyl * dy + (tu0 << NM3D_FIXP16_SHIFT);
				vl = dvdyl * dy + (tv0 << NM3D_FIXP16_SHIFT);
				wl = dwdyl * dy + (tw0 << NM3D_FIXP16_SHIFT);

				// compute new RHS starting values
				xr = dxdyr * dy + (x0 << NM3D_FIXP16_SHIFT);
				ur = dudyr * dy + (tu0 << NM3D_FIXP16_SHIFT);
				vr = dvdyr * dy + (tv0 << NM3D_FIXP16_SHIFT);
				wr = dwdyr * dy + (tw0 << NM3D_FIXP16_SHIFT);

				// compute new starting y
				ystart = min_clip_y;

			} // end if
			else
			{
				// no clipping

				// set starting values
				xl = (x0 << NM3D_FIXP16_SHIFT);
				xr = (x0 << NM3D_FIXP16_SHIFT);

				ul = (tu0 << NM3D_FIXP16_SHIFT);
				vl = (tv0 << NM3D_FIXP16_SHIFT);
				wl = (tw0 << NM3D_FIXP16_SHIFT);

				ur = (tu0 << NM3D_FIXP16_SHIFT);
				vr = (tv0 << NM3D_FIXP16_SHIFT);
				wr = (tw0 << NM3D_FIXP16_SHIFT);

				// set starting y
				ystart = y0;

			} // end else

		} // end else flat bottom

		// test for bottom clip, always
		if ((yend = y2) > max_clip_y)
			yend = max_clip_y;

		// test for horizontal clipping
		if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
			(x1 < min_clip_x) || (x1 > max_clip_x) ||
			(x2 < min_clip_x) || (x2 > max_clip_x))
		{
			// clip version

			// point screen ptr to starting line
			screen_ptr = dest_buffer + (ystart * mem_pitch);

			for (yi = ystart; yi < yend; yi++)
			{
				// compute span endpoints
				xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
				xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

				// compute starting points for u,v,w interpolants
				ui = ul + NM3D_FIXP16_ROUND_UP;
				vi = vl + NM3D_FIXP16_ROUND_UP;
				wi = wl + NM3D_FIXP16_ROUND_UP;

				// compute u,v interpolants
				if ((dx = (xend - xstart)) > 0)
				{
					du = (ur - ul) / dx;
					dv = (vr - vl) / dx;
					dw = (wr - wl) / dx;
				} // end if
				else
				{
					du = (ur - ul);
					dv = (vr - vl);
					dw = (wr - wl);
				} // end else

				///////////////////////////////////////////////////////////////////////

				// test for x clipping, LHS
				if (xstart < min_clip_x)
				{
					// compute x overlap
					dx = min_clip_x - xstart;

					// slide interpolants over
					ui += dx * du;
					vi += dx * dv;
					wi += dx * dw;

					// reset vars
					xstart = min_clip_x;

				} // end if

				// test for x clipping RHS
				if (xend > max_clip_x)
					xend = max_clip_x;

				///////////////////////////////////////////////////////////////////////

				// draw span
				for (xi = xstart; xi < xend; xi++)
				{
					// write textel assume 5.6.5
#ifdef NM3D_PIXELFORMAT_565
					screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 11) + ((vi >> (NM3D_FIXP16_SHIFT + 2)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#else
					screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 10) + ((vi >> (NM3D_FIXP16_SHIFT + 3)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#endif
					// screen_ptr[xi] = CRGBA(ui,vi,wi).GetRGB();
					//  interpolate u,v
					ui += du;
					vi += dv;
					wi += dw;
				} // end for xi

				// interpolate u,v,w,x along right and left edge
				xl += dxdyl;
				ul += dudyl;
				vl += dvdyl;
				wl += dwdyl;

				xr += dxdyr;
				ur += dudyr;
				vr += dvdyr;
				wr += dwdyr;

				// advance screen ptr
				screen_ptr += mem_pitch;

			} // end for y

		} // end if clip
		else
		{
			// non-clip version

			// point screen ptr to starting line
			screen_ptr = dest_buffer + (ystart * mem_pitch);

			for (yi = ystart; yi < yend; yi++)
			{
				// compute span endpoints
				xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
				xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

				// compute starting points for u,v,w interpolants
				ui = ul + NM3D_FIXP16_ROUND_UP;
				vi = vl + NM3D_FIXP16_ROUND_UP;
				wi = wl + NM3D_FIXP16_ROUND_UP;

				// compute u,v interpolants
				if ((dx = (xend - xstart)) > 0)
				{
					du = (ur - ul) / dx;
					dv = (vr - vl) / dx;
					dw = (wr - wl) / dx;
				} // end if
				else
				{
					du = (ur - ul);
					dv = (vr - vl);
					dw = (wr - wl);
				} // end else

				// draw span
				for (xi = xstart; xi < xend; xi++)
				{
					// write textel 5.6.5
#ifdef NM3D_PIXELFORMAT_565
					screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 11) + ((vi >> (NM3D_FIXP16_SHIFT + 2)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#else
					screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 10) + ((vi >> (NM3D_FIXP16_SHIFT + 3)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#endif
					// screen_ptr[xi] = CRGBA(ui,vi,wi).GetRGB();
					//  interpolate u,v
					ui += du;
					vi += dv;
					wi += dw;
				} // end for xi

				// interpolate u,v,w,x along right and left edge
				xl += dxdyl;
				ul += dudyl;
				vl += dvdyl;
				wl += dwdyl;

				xr += dxdyr;
				ur += dudyr;
				vr += dvdyr;
				wr += dwdyr;

				// advance screen ptr
				screen_ptr += mem_pitch;

			} // end for y

		} // end if non-clipped

	} // end if
	else
	{
		if (tri_type == NM3D_TRI_TYPE_GENERAL)
		{

			// first test for bottom clip, always
			if ((yend = y2) > max_clip_y)
				yend = max_clip_y;

			// pre-test y clipping status
			if (y1 < min_clip_y)
			{
				// compute all deltas
				// LHS
				dyl = (y2 - y1);

				dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;
				dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dyl;
				dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dyl;
				dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dyl;

				// RHS
				dyr = (y2 - y0);

				dxdyr = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dyr;
				dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT) / dyr;
				dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT) / dyr;
				dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT) / dyr;

				// compute overclip
				dyr = (min_clip_y - y0);
				dyl = (min_clip_y - y1);

				// computer new LHS starting values
				xl = dxdyl * dyl + (x1 << NM3D_FIXP16_SHIFT);

				ul = dudyl * dyl + (tu1 << NM3D_FIXP16_SHIFT);
				vl = dvdyl * dyl + (tv1 << NM3D_FIXP16_SHIFT);
				wl = dwdyl * dyl + (tw1 << NM3D_FIXP16_SHIFT);

				// compute new RHS starting values
				xr = dxdyr * dyr + (x0 << NM3D_FIXP16_SHIFT);

				ur = dudyr * dyr + (tu0 << NM3D_FIXP16_SHIFT);
				vr = dvdyr * dyr + (tv0 << NM3D_FIXP16_SHIFT);
				wr = dwdyr * dyr + (tw0 << NM3D_FIXP16_SHIFT);

				// compute new starting y
				ystart = min_clip_y;

				// test if we need swap to keep rendering left to right
				if (dxdyr > dxdyl)
				{
					SWAP(dxdyl, dxdyr /*,temp*/);
					SWAP(dudyl, dudyr /*,temp*/);
					SWAP(dvdyl, dvdyr /*,temp*/);
					SWAP(dwdyl, dwdyr /*,temp*/);
					SWAP(xl, xr /*,temp*/);
					SWAP(ul, ur /*,temp*/);
					SWAP(vl, vr /*,temp*/);
					SWAP(wl, wr /*,temp*/);
					SWAP(x1, x2 /*,temp*/);
					SWAP(y1, y2 /*,temp*/);
					SWAP(tu1, tu2 /*,temp*/);
					SWAP(tv1, tv2 /*,temp*/);
					SWAP(tw1, tw2 /*,temp*/);

					// set interpolation restart
					irestart = NM3D_INTERP_RHS;

				} // end if

			} // end if
			else
			{
				if (y0 < min_clip_y)
				{
					// compute all deltas
					// LHS
					dyl = (y1 - y0);

					dxdyl = ((x1 - x0) << NM3D_FIXP16_SHIFT) / dyl;
					dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT) / dyl;
					dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT) / dyl;
					dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT) / dyl;

					// RHS
					dyr = (y2 - y0);

					dxdyr = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dyr;
					dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT) / dyr;
					dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT) / dyr;
					dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT) / dyr;

					// compute overclip
					dy = (min_clip_y - y0);

					// computer new LHS starting values
					xl = dxdyl * dy + (x0 << NM3D_FIXP16_SHIFT);
					ul = dudyl * dy + (tu0 << NM3D_FIXP16_SHIFT);
					vl = dvdyl * dy + (tv0 << NM3D_FIXP16_SHIFT);
					wl = dwdyl * dy + (tw0 << NM3D_FIXP16_SHIFT);

					// compute new RHS starting values
					xr = dxdyr * dy + (x0 << NM3D_FIXP16_SHIFT);
					ur = dudyr * dy + (tu0 << NM3D_FIXP16_SHIFT);
					vr = dvdyr * dy + (tv0 << NM3D_FIXP16_SHIFT);
					wr = dwdyr * dy + (tw0 << NM3D_FIXP16_SHIFT);

					// compute new starting y
					ystart = min_clip_y;

					// test if we need swap to keep rendering left to right
					if (dxdyr < dxdyl)
					{
						SWAP(dxdyl, dxdyr /*,temp*/);
						SWAP(dudyl, dudyr /*,temp*/);
						SWAP(dvdyl, dvdyr /*,temp*/);
						SWAP(dwdyl, dwdyr /*,temp*/);
						SWAP(xl, xr /*,temp*/);
						SWAP(ul, ur /*,temp*/);
						SWAP(vl, vr /*,temp*/);
						SWAP(wl, wr /*,temp*/);
						SWAP(x1, x2 /*,temp*/);
						SWAP(y1, y2 /*,temp*/);
						SWAP(tu1, tu2 /*,temp*/);
						SWAP(tv1, tv2 /*,temp*/);
						SWAP(tw1, tw2 /*,temp*/);

						// set interpolation restart
						irestart = NM3D_INTERP_RHS;

					} // end if

				} // end if
				else
				{
					// no initial y clipping

					// compute all deltas
					// LHS
					dyl = (y1 - y0);

					dxdyl = ((x1 - x0) << NM3D_FIXP16_SHIFT) / dyl;
					dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT) / dyl;
					dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT) / dyl;
					dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT) / dyl;

					// RHS
					dyr = (y2 - y0);

					dxdyr = ((x2 - x0) << NM3D_FIXP16_SHIFT) / dyr;
					dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT) / dyr;
					dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT) / dyr;
					dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT) / dyr;

					// no clipping y

					// set starting values
					xl = (x0 << NM3D_FIXP16_SHIFT);
					xr = (x0 << NM3D_FIXP16_SHIFT);

					ul = (tu0 << NM3D_FIXP16_SHIFT);
					vl = (tv0 << NM3D_FIXP16_SHIFT);
					wl = (tw0 << NM3D_FIXP16_SHIFT);

					ur = (tu0 << NM3D_FIXP16_SHIFT);
					vr = (tv0 << NM3D_FIXP16_SHIFT);
					wr = (tw0 << NM3D_FIXP16_SHIFT);

					// set starting y
					ystart = y0;

					// test if we need swap to keep rendering left to right
					if (dxdyr < dxdyl)
					{
						SWAP(dxdyl, dxdyr /*,temp*/);
						SWAP(dudyl, dudyr /*,temp*/);
						SWAP(dvdyl, dvdyr /*,temp*/);
						SWAP(dwdyl, dwdyr /*,temp*/);
						SWAP(xl, xr /*,temp*/);
						SWAP(ul, ur /*,temp*/);
						SWAP(vl, vr /*,temp*/);
						SWAP(wl, wr /*,temp*/);
						SWAP(x1, x2 /*,temp*/);
						SWAP(y1, y2 /*,temp*/);
						SWAP(tu1, tu2 /*,temp*/);
						SWAP(tv1, tv2 /*,temp*/);
						SWAP(tw1, tw2 /*,temp*/);

						// set interpolation restart
						irestart = NM3D_INTERP_RHS;

					} // end if

				} // end else
			}

			// test for horizontal clipping
			if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
				(x1 < min_clip_x) || (x1 > max_clip_x) ||
				(x2 < min_clip_x) || (x2 > max_clip_x))
			{
				// clip version
				// x clipping

				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					// compute starting points for u,v,w interpolants
					ui = ul + NM3D_FIXP16_ROUND_UP;
					vi = vl + NM3D_FIXP16_ROUND_UP;
					wi = wl + NM3D_FIXP16_ROUND_UP;

					// compute u,v interpolants
					if ((dx = (xend - xstart)) > 0)
					{
						du = (ur - ul) / dx;
						dv = (vr - vl) / dx;
						dw = (wr - wl) / dx;
					} // end if
					else
					{
						du = (ur - ul);
						dv = (vr - vl);
						dw = (wr - wl);
					} // end else

					///////////////////////////////////////////////////////////////////////

					// test for x clipping, LHS
					if (xstart < min_clip_x)
					{
						// compute x overlap
						dx = min_clip_x - xstart;

						// slide interpolants over
						ui += dx * du;
						vi += dx * dv;
						wi += dx * dw;

						// set x to left clip edge
						xstart = min_clip_x;

					} // end if

					// test for x clipping RHS
					if (xend > max_clip_x)
						xend = max_clip_x;

					///////////////////////////////////////////////////////////////////////

					// draw span
					for (xi = xstart; xi < xend; xi++)
					{
						// write textel assume 5.6.5
#ifdef NM3D_PIXELFORMAT_565
						screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 11) + ((vi >> (NM3D_FIXP16_SHIFT + 2)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#else
						screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 10) + ((vi >> (NM3D_FIXP16_SHIFT + 3)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#endif
						// screen_ptr[xi] = CRGBA(ui,vi,wi).GetRGB();
						//  interpolate u,v
						ui += du;
						vi += dv;
						wi += dw;
					} // end for xi

					// interpolate u,v,w,x along right and left edge
					xl += dxdyl;
					ul += dudyl;
					vl += dvdyl;
					wl += dwdyl;

					xr += dxdyr;
					ur += dudyr;
					vr += dvdyr;
					wr += dwdyr;

					// advance screen ptr
					screen_ptr += mem_pitch;

					// test for yi hitting second region, if so change interpolant
					if (yi == yrestart)
					{
						// test interpolation side change flag

						if (irestart == NM3D_INTERP_LHS)
						{
							// LHS
							dyl = (y2 - y1);

							dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;
							dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dyl;
							dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dyl;
							dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dyl;

							// set starting values
							xl = (x1 << NM3D_FIXP16_SHIFT);
							ul = (tu1 << NM3D_FIXP16_SHIFT);
							vl = (tv1 << NM3D_FIXP16_SHIFT);
							wl = (tw1 << NM3D_FIXP16_SHIFT);

							// interpolate down on LHS to even up
							xl += dxdyl;
							ul += dudyl;
							vl += dvdyl;
							wl += dwdyl;
						} // end if
						else
						{
							// RHS
							dyr = (y1 - y2);

							dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;
							dudyr = ((tu1 - tu2) << NM3D_FIXP16_SHIFT) / dyr;
							dvdyr = ((tv1 - tv2) << NM3D_FIXP16_SHIFT) / dyr;
							dwdyr = ((tw1 - tw2) << NM3D_FIXP16_SHIFT) / dyr;

							// set starting values
							xr = (x2 << NM3D_FIXP16_SHIFT);
							ur = (tu2 << NM3D_FIXP16_SHIFT);
							vr = (tv2 << NM3D_FIXP16_SHIFT);
							wr = (tw2 << NM3D_FIXP16_SHIFT);

							// interpolate down on RHS to even up
							xr += dxdyr;
							ur += dudyr;
							vr += dvdyr;
							wr += dwdyr;

						} // end else

					} // end if

				} // end for y

			} // end if
			else
			{
				// no x clipping
				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					// compute starting points for u,v,w interpolants
					ui = ul + NM3D_FIXP16_ROUND_UP;
					vi = vl + NM3D_FIXP16_ROUND_UP;
					wi = wl + NM3D_FIXP16_ROUND_UP;

					// compute u,v interpolants
					if ((dx = (xend - xstart)) > 0)
					{
						du = (ur - ul) / dx;
						dv = (vr - vl) / dx;
						dw = (wr - wl) / dx;
					} // end if
					else
					{
						du = (ur - ul);
						dv = (vr - vl);
						dw = (wr - wl);
					} // end else

					// draw span
					for (xi = xstart; xi < xend; xi++)
					{
						// write textel assume 5.6.5
#ifdef NM3D_PIXELFORMAT_565
						screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 11) + ((vi >> (NM3D_FIXP16_SHIFT + 2)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#else
						screen_ptr[xi] = ((ui >> (NM3D_FIXP16_SHIFT + 3)) << 10) + ((vi >> (NM3D_FIXP16_SHIFT + 3)) << 5) + (wi >> (NM3D_FIXP16_SHIFT + 3));
#endif
						// screen_ptr[xi] = CRGBA(ui,vi,wi).GetRGB();
						//  interpolate u,v
						ui += du;
						vi += dv;
						wi += dw;
					} // end for xi

					// interpolate u,v,w,x along right and left edge
					xl += dxdyl;
					ul += dudyl;
					vl += dvdyl;
					wl += dwdyl;

					xr += dxdyr;
					ur += dudyr;
					vr += dvdyr;
					wr += dwdyr;

					// advance screen ptr
					screen_ptr += mem_pitch;

					// test for yi hitting second region, if so change interpolant
					if (yi == yrestart)
					{
						// test interpolation side change flag

						if (irestart == NM3D_INTERP_LHS)
						{
							// LHS
							dyl = (y2 - y1);

							dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;
							dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dyl;
							dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dyl;
							dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dyl;

							// set starting values
							xl = (x1 << NM3D_FIXP16_SHIFT);
							ul = (tu1 << NM3D_FIXP16_SHIFT);
							vl = (tv1 << NM3D_FIXP16_SHIFT);
							wl = (tw1 << NM3D_FIXP16_SHIFT);

							// interpolate down on LHS to even up
							xl += dxdyl;
							ul += dudyl;
							vl += dvdyl;
							wl += dwdyl;
						} // end if
						else
						{
							// RHS
							dyr = (y1 - y2);

							dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;
							dudyr = ((tu1 - tu2) << NM3D_FIXP16_SHIFT) / dyr;
							dvdyr = ((tv1 - tv2) << NM3D_FIXP16_SHIFT) / dyr;
							dwdyr = ((tw1 - tw2) << NM3D_FIXP16_SHIFT) / dyr;

							// set starting values
							xr = (x2 << NM3D_FIXP16_SHIFT);
							ur = (tu2 << NM3D_FIXP16_SHIFT);
							vr = (tv2 << NM3D_FIXP16_SHIFT);
							wr = (tw2 << NM3D_FIXP16_SHIFT);

							// interpolate down on RHS to even up
							xr += dxdyr;
							ur += dudyr;
							vr += dvdyr;
							wr += dwdyr;
						} // end else

					} // end if

				} // end for y

			} // end else

		} // end if
	}
} // end Draw_Gouraud_Triangle2_16

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//Gouraud texture
#if 1
void Nomad3D::CRender::Draw_Textured_TriangleGS_16(float poly[3][3], float uu[3],float vv[3],unsigned short *buff)       // bytes per line, 320, 640 etc.
{
	// this function draws a textured gouraud shaded polygon, based on the affine texture mapper, 
	// we simply interpolate the (R,G,B) values across the polygons along with the texture coordinates
	// and then modulate to get the final color 
	(void)vv;
	(void)uu;
	int v0=0,
		v1=1,
		v2=2,
		//temp=0,
		tri_type = NM3D_TRI_TYPE_NONE,
		irestart = NM3D_INTERP_LHS;

	int dx,dy,dyl,dyr,      // general deltas
//		u,v,w, s,t,
		du,dv,dw, ds, dt, 
		xi,yi,              // the current interpolated x,y
		ui,vi,wi, si, ti,    // the current interpolated u,v
//		index_x,index_y,    // looping vars
//		x,y,                // hold general x,y
		xstart,
		xend,
		ystart,
		yrestart,
		yend,
		xl,                 
		dxdyl,              
		xr,
		dxdyr,             
		dudyl,    
		ul,
		dvdyl,   
		vl,
		dwdyl,   
		wl,
		dsdyl,    
		sl,
		dtdyl,   
		tl,
		dudyr,
		ur,
		dvdyr,
		vr,
		dwdyr,
		wr,
		dsdyr,
		sr,
		dtdyr,
		tr;

	int x0,y0,tu0,tv0,tw0, ts0,tt0,    // cached vertices
		x1,y1,tu1,tv1,tw1, ts1,tt1,
		x2,y2,tu2,tv2,tw2, ts2,tt2;

	int r_base0, g_base0, b_base0,
		r_base1, g_base1, b_base1,
		r_base2, g_base2, b_base2;


	UINT r_textel, g_textel, b_textel;
	USHORT textel;

	USHORT *screen_ptr  = NULL,
		//*screen_line = NULL,
		*textmap     = NULL,
		*dest_buffer = (USHORT *)buff;

#ifdef DEBUG_ON
	// track rendering stats
	debug_polys_rendered_per_frame++;
#endif


	// extract texture map
	textmap = (USHORT *)(m_pMaterial->m_imgTexture.GetData());
	if(!textmap)
	{
		GouraudShading(poly,buff);
		return;
	}

	// extract base 2 of texture width
	int texture_shift2 = Log2(m_pMaterial->m_imgTexture.GetWidth());

	// adjust memory pitch to words, divide by 2
	int mem_pitch = m_nMemPitch>>1;

	// apply fill convention to coordinates
	poly[0][0] = (int)(poly[0][0]+0.0);
	poly[0][1] = (int)(poly[0][1]+0.0);

	poly[1][0] = (int)(poly[1][0]+0.0);
	poly[1][1] = (int)(poly[1][1]+0.0);

	poly[2][0] = (int)(poly[2][0]+0.0);
	poly[2][1] = (int)(poly[2][1]+0.0);

	// first trivial clipping rejection tests 
	if (((poly[0][1] < min_clip_y)  && 
		(poly[1][1] < min_clip_y)  &&
		(poly[2][1] < min_clip_y)) ||

		((poly[0][1] > max_clip_y)  && 
		(poly[1][1] > max_clip_y)  &&
		(poly[2][1] > max_clip_y)) ||

		((poly[0][0] < min_clip_x)  && 
		(poly[1][0] < min_clip_x)  &&
		(poly[2][0] < min_clip_x)) ||

		((poly[0][0] > max_clip_x)  && 
		(poly[1][0] > max_clip_x)  &&
		(poly[2][0] > max_clip_x)))
		return;

	// sort vertices
	if (poly[v1][1] < poly[v0][1]) 
	{SWAP(v0,v1/*,temp*/);} 

	if (poly[v2][1] < poly[v0][1]) 
	{SWAP(v0,v2/*,temp*/);}

	if (poly[v2][1] < poly[v1][1]) 
	{SWAP(v1,v2/*,temp*/);}

	// now test for trivial flat sided cases
	if (FCMP(poly[v0][1], poly[v1][1]) )
	{ 
		// set triangle type
		tri_type = NM3D_TRI_TYPE_FLAT_TOP;

		// sort vertices left to right
		if (poly[v1][0] < poly[v0][0]) 
		{SWAP(v0,v1/*,temp*/);}

	} // end if
	else
		// now test for trivial flat sided cases
		if (FCMP(poly[v1][1], poly[v2][1]) )
		{ 
			// set triangle type
			tri_type = NM3D_TRI_TYPE_FLAT_BOTTOM;

			// sort vertices left to right
			if (poly[v2][0] < poly[v1][0]) 
			{SWAP(v1,v2/*,temp*/);}

		} // end if
		else
		{
			// must be a general triangle
			tri_type = NM3D_TRI_TYPE_GENERAL;

		} // end else

		// assume 5.6.5 format -- sorry!
		// we can't afford a function call in the inner loops, so we must write 
		// two hard coded versions, if we want support for both 5.6.5, and 5.5.5
// #ifdef PIXEL_FORMAT565
// 		_RGB565FROM16BIT(face->lit_color[v0], &r_base0, &g_base0, &b_base0);
// 		_RGB565FROM16BIT(face->lit_color[v1], &r_base1, &g_base1, &b_base1);
// 		_RGB565FROM16BIT(face->lit_color[v2], &r_base2, &g_base2, &b_base2);
// 
// 		// scale to 8 bit 
// 		r_base0 <<= 3;
// 		g_base0 <<= 2;
// 		b_base0 <<= 3;
// 
// 		// scale to 8 bit 
// 		r_base1 <<= 3;
// 		g_base1 <<= 2;
// 		b_base1 <<= 3;
// 
// 		// scale to 8 bit 
// 		r_base2 <<= 3;
// 		g_base2 <<= 2;
// 		b_base2 <<= 3;
// #else //PIXEL_FORMAT555
// 		_RGB555FROM16BIT(face->lit_color[v0], &r_base0, &g_base0, &b_base0);
// 		_RGB555FROM16BIT(face->lit_color[v1], &r_base1, &g_base1, &b_base1);
// 		_RGB555FROM16BIT(face->lit_color[v2], &r_base2, &g_base2, &b_base2);
// 
// 		// scale to 8 bit 
// 		r_base0 <<= 3;
// 		g_base0 <<= 3;
// 		b_base0 <<= 3;
// 
// 		// scale to 8 bit 
// 		r_base1 <<= 3;
// 		g_base1 <<= 3;
// 		b_base1 <<= 3;
// 
// 		// scale to 8 bit 
// 		r_base2 <<= 3;
// 		g_base2 <<= 3;
// 		b_base2 <<= 3;
// #endif
		m_rgba[v0].GetRGBValuec(r_base0,g_base0,b_base0);
		m_rgba[v1].GetRGBValuec(r_base1,g_base1,b_base1);
		m_rgba[v2].GetRGBValuec(r_base2,g_base2,b_base2);


		// extract vertices for processing, now that we have order
		x0  = (int)(poly[v0][0]+0.0);
		y0  = (int)(poly[v0][1]+0.0);

		ts0 = (int)(m_fUCoords[v0]);
		tt0 = (int)(m_fVCoords[v0]);

		tu0 = r_base0;
		tv0 = g_base0; 
		tw0 = b_base0; 

		x1  = (int)(poly[v1][0]+0.0);
		y1  = (int)(poly[v1][1]+0.0);

		ts1 = (int)(m_fUCoords[v1]);
		tt1 = (int)(m_fVCoords[v1]);

		tu1 = r_base1;
		tv1 = g_base1; 
		tw1 = b_base1; 

		x2  = (int)(poly[v2][0]+0.0);
		y2  = (int)(poly[v2][1]+0.0);

		ts2 = (int)(m_fUCoords[v2]);
		tt2 = (int)(m_fVCoords[v2]);

		tu2 = r_base2; 
		tv2 = g_base2; 
		tw2 = b_base2; 


		// degenerate triangle
		if ( ((x0 == x1) && (x1 == x2)) || ((y0 ==  y1) && (y1 == y2)))
			return;

		// set interpolation restart value
		yrestart = y1;

		// what kind of triangle
		if (tri_type & NM3D_TRI_TYPE_FLAT_MASK)
		{

			if (tri_type == NM3D_TRI_TYPE_FLAT_TOP)
			{
				// compute all deltas
				dy = (y2 - y0);

				dxdyl = ((x2 - x0)   << NM3D_FIXP16_SHIFT)/dy;
				dudyl = ((tu2 - tu0) << NM3D_FIXP16_SHIFT)/dy;  
				dvdyl = ((tv2 - tv0) << NM3D_FIXP16_SHIFT)/dy;    
				dwdyl = ((tw2 - tw0) << NM3D_FIXP16_SHIFT)/dy;  

				dsdyl = ((ts2 - ts0) << NM3D_FIXP16_SHIFT)/dy;    
				dtdyl = ((tt2 - tt0) << NM3D_FIXP16_SHIFT)/dy;  

				dxdyr = ((x2 - x1)   << NM3D_FIXP16_SHIFT)/dy;
				dudyr = ((tu2 - tu1) << NM3D_FIXP16_SHIFT)/dy;  
				dvdyr = ((tv2 - tv1) << NM3D_FIXP16_SHIFT)/dy;   
				dwdyr = ((tw2 - tw1) << NM3D_FIXP16_SHIFT)/dy;   

				dsdyr = ((ts2 - ts1) << NM3D_FIXP16_SHIFT)/dy;   
				dtdyr = ((tt2 - tt1) << NM3D_FIXP16_SHIFT)/dy;   


				// test for y clipping
				if (y0 < min_clip_y)
				{
					// compute overclip
					dy = (min_clip_y - y0);

					// computer new LHS starting values
					xl = dxdyl*dy + (x0  << NM3D_FIXP16_SHIFT);
					ul = dudyl*dy + (tu0 << NM3D_FIXP16_SHIFT);
					vl = dvdyl*dy + (tv0 << NM3D_FIXP16_SHIFT);
					wl = dwdyl*dy + (tw0 << NM3D_FIXP16_SHIFT);

					sl = dsdyl*dy + (ts0 << NM3D_FIXP16_SHIFT);
					tl = dtdyl*dy + (tt0 << NM3D_FIXP16_SHIFT);


					// compute new RHS starting values
					xr = dxdyr*dy + (x1  << NM3D_FIXP16_SHIFT);
					ur = dudyr*dy + (tu1 << NM3D_FIXP16_SHIFT);
					vr = dvdyr*dy + (tv1 << NM3D_FIXP16_SHIFT);
					wr = dwdyr*dy + (tw1 << NM3D_FIXP16_SHIFT);

					sr = dsdyr*dy + (ts1 << NM3D_FIXP16_SHIFT);
					tr = dtdyr*dy + (tt1 << NM3D_FIXP16_SHIFT);

					// compute new starting y
					ystart = min_clip_y;

				} // end if
				else
				{
					// no clipping

					// set starting values
					xl = (x0 << NM3D_FIXP16_SHIFT);
					xr = (x1 << NM3D_FIXP16_SHIFT);

					ul = (tu0 << NM3D_FIXP16_SHIFT);
					vl = (tv0 << NM3D_FIXP16_SHIFT);
					wl = (tw0 << NM3D_FIXP16_SHIFT);

					sl = (ts0 << NM3D_FIXP16_SHIFT);
					tl = (tt0 << NM3D_FIXP16_SHIFT);


					ur = (tu1 << NM3D_FIXP16_SHIFT);
					vr = (tv1 << NM3D_FIXP16_SHIFT);
					wr = (tw1 << NM3D_FIXP16_SHIFT);

					sr = (ts1 << NM3D_FIXP16_SHIFT);
					tr = (tt1 << NM3D_FIXP16_SHIFT);

					// set starting y
					ystart = y0;

				} // end else

			} // end if flat top
			else
			{
				// must be flat bottom

				// compute all deltas
				dy = (y1 - y0);

				dxdyl = ((x1 - x0)   << NM3D_FIXP16_SHIFT)/dy;
				dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT)/dy;  
				dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT)/dy;    
				dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT)/dy; 

				dsdyl = ((ts1 - ts0) << NM3D_FIXP16_SHIFT)/dy;    
				dtdyl = ((tt1 - tt0) << NM3D_FIXP16_SHIFT)/dy; 


				dxdyr = ((x2 - x0)   << NM3D_FIXP16_SHIFT)/dy;
				dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT)/dy;  
				dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT)/dy;   
				dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT)/dy;   

				dsdyr = ((ts2 - ts0) << NM3D_FIXP16_SHIFT)/dy;   
				dtdyr = ((tt2 - tt0) << NM3D_FIXP16_SHIFT)/dy;   


				// test for y clipping
				if (y0 < min_clip_y)
				{
					// compute overclip
					dy = (min_clip_y - y0);

					// computer new LHS starting values
					xl = dxdyl*dy + (x0  << NM3D_FIXP16_SHIFT);
					ul = dudyl*dy + (tu0 << NM3D_FIXP16_SHIFT);
					vl = dvdyl*dy + (tv0 << NM3D_FIXP16_SHIFT);
					wl = dwdyl*dy + (tw0 << NM3D_FIXP16_SHIFT);

					sl = dsdyl*dy + (ts0 << NM3D_FIXP16_SHIFT);
					tl = dtdyl*dy + (tt0 << NM3D_FIXP16_SHIFT);


					// compute new RHS starting values
					xr = dxdyr*dy + (x0  << NM3D_FIXP16_SHIFT);
					ur = dudyr*dy + (tu0 << NM3D_FIXP16_SHIFT);
					vr = dvdyr*dy + (tv0 << NM3D_FIXP16_SHIFT);
					wr = dwdyr*dy + (tw0 << NM3D_FIXP16_SHIFT);

					sr = dsdyr*dy + (ts0 << NM3D_FIXP16_SHIFT);
					tr = dtdyr*dy + (tt0 << NM3D_FIXP16_SHIFT);


					// compute new starting y
					ystart = min_clip_y;

				} // end if
				else
				{
					// no clipping

					// set starting values
					xl = (x0 << NM3D_FIXP16_SHIFT);
					xr = (x0 << NM3D_FIXP16_SHIFT);

					ul = (tu0 << NM3D_FIXP16_SHIFT);
					vl = (tv0 << NM3D_FIXP16_SHIFT);
					wl = (tw0 << NM3D_FIXP16_SHIFT);

					sl = (ts0 << NM3D_FIXP16_SHIFT);
					tl = (tt0 << NM3D_FIXP16_SHIFT);


					ur = (tu0 << NM3D_FIXP16_SHIFT);
					vr = (tv0 << NM3D_FIXP16_SHIFT);
					wr = (tw0 << NM3D_FIXP16_SHIFT);

					sr = (ts0 << NM3D_FIXP16_SHIFT);
					tr = (tt0 << NM3D_FIXP16_SHIFT);


					// set starting y
					ystart = y0;

				} // end else	

			} // end else flat bottom

			// test for bottom clip, always
			if ((yend = y2) > max_clip_y)
				yend = max_clip_y;

			// test for horizontal clipping
			if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
				(x1 < min_clip_x) || (x1 > max_clip_x) ||
				(x2 < min_clip_x) || (x2 > max_clip_x))
			{
				// clip version

				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend   = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					// compute starting points for u,v,w interpolants
					ui = ul + NM3D_FIXP16_ROUND_UP;
					vi = vl + NM3D_FIXP16_ROUND_UP;
					wi = wl + NM3D_FIXP16_ROUND_UP;

					si = sl + NM3D_FIXP16_ROUND_UP;
					ti = tl + NM3D_FIXP16_ROUND_UP;

					// compute u,v interpolants
					if ((dx = (xend - xstart))>0)
					{
						du = (ur - ul)/dx;
						dv = (vr - vl)/dx;
						dw = (wr - wl)/dx;

						ds = (sr - sl)/dx;
						dt = (tr - tl)/dx;

					} // end if
					else
					{
						du = (ur - ul);
						dv = (vr - vl);
						dw = (wr - wl);

						ds = (sr - sl);
						dt = (tr - tl);

					} // end else

					///////////////////////////////////////////////////////////////////////

					// test for x clipping, LHS
					if (xstart < min_clip_x)
					{
						// compute x overlap
						dx = min_clip_x - xstart;

						// slide interpolants over
						ui+=dx*du;
						vi+=dx*dv;
						wi+=dx*dw;

						si+=dx*ds;
						ti+=dx*dt;

						// reset vars
						xstart = min_clip_x;

					} // end if

					// test for x clipping RHS
					if (xend > max_clip_x)
						xend = max_clip_x;

					///////////////////////////////////////////////////////////////////////

					// draw span
					for (xi=xstart; xi < xend; xi++)
					{
						// write textel assume 5.6.5


						// get textel first
						textel = textmap[(si >> NM3D_FIXP16_SHIFT) + ((ti >> NM3D_FIXP16_SHIFT) << texture_shift2)];
#ifdef NM3D_PIXELFORMAT_565
						// extract rgb components
						r_textel  = ((textel >> 11)       ); 
						g_textel  = ((textel >> 5)  & 0x3f); 
						b_textel =   (textel        & 0x1f);
#else //PIXEL_FORMAT555
						r_textel  = ((textel >> 10) & 0x1f); 
						g_textel  = ((textel >> 5)  & 0x1f); 
						b_textel =   (textel        & 0x1f);
#endif
						// modulate textel with gouraud shading
						r_textel*=ui; 
						g_textel*=vi;
						b_textel*=wi;

						// finally write pixel, note that we did the math such that the results are r*32, g*64, b*32
						// hence we need to divide the results by 32,64,32 respetively, BUT since we need to shift
						// the results to fit into the destination 5.6.5 word, we can take advantage of the shifts
						// and they all cancel out for the most part, but we will need logical anding, we will do
						// it later when we optimize more...
#ifdef NM3D_PIXELFORMAT_565
						screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT+8)) + 
							((g_textel >> (NM3D_FIXP16_SHIFT+8)) << 5) + 
							((r_textel >> (NM3D_FIXP16_SHIFT+8)) << 11));
#else //PIXEL_FORMAT555
						screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT+8)) + 
							((g_textel >> (NM3D_FIXP16_SHIFT+8)) << 5) + 
							((r_textel >> (NM3D_FIXP16_SHIFT+8)) << 10));
#endif 
						// interpolate u,v
						ui+=du;
						vi+=dv;
						wi+=dw;

						si+=ds;
						ti+=dt;

					} // end for xi

					// interpolate u,v,w,x along right and left edge
					xl+=dxdyl;
					ul+=dudyl;
					vl+=dvdyl;
					wl+=dwdyl;

					sl+=dsdyl;
					tl+=dtdyl;

					xr+=dxdyr;
					ur+=dudyr;
					vr+=dvdyr;
					wr+=dwdyr;

					sr+=dsdyr;
					tr+=dtdyr;


					// advance screen ptr
					screen_ptr+=mem_pitch;

				} // end for y

			} // end if clip
			else
			{
				// non-clip version

				// point screen ptr to starting line
				screen_ptr = dest_buffer + (ystart * mem_pitch);

				for (yi = ystart; yi < yend; yi++)
				{
					// compute span endpoints
					xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
					xend   = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

					// compute starting points for u,v,w interpolants
					ui = ul + NM3D_FIXP16_ROUND_UP;
					vi = vl + NM3D_FIXP16_ROUND_UP;
					wi = wl + NM3D_FIXP16_ROUND_UP;

					si = sl + NM3D_FIXP16_ROUND_UP;
					ti = tl + NM3D_FIXP16_ROUND_UP;

					// compute u,v interpolants
					if ((dx = (xend - xstart))>0)
					{
						du = (ur - ul)/dx;
						dv = (vr - vl)/dx;
						dw = (wr - wl)/dx;

						ds = (sr - sl)/dx;
						dt = (tr - tl)/dx;

					} // end if
					else
					{
						du = (ur - ul);
						dv = (vr - vl);
						dw = (wr - wl);

						ds = (sr - sl);
						dt = (tr - tl);

					} // end else

					// draw span
					for (xi=xstart; xi < xend; xi++)
					{
						// write textel assume 5.6.5
						//screen_ptr[xi] = ( (ui >> (FIXP16_SHIFT+3)) << 11) + 
						//                 ( (vi >> (FIXP16_SHIFT+2)) << 5) + 
						//                   (wi >> (FIXP16_SHIFT+3) );   

						// get textel first
						textel = textmap[(si >> NM3D_FIXP16_SHIFT) + ((ti >> NM3D_FIXP16_SHIFT) << texture_shift2)];
#ifdef NM3D_PIXELFORMAT_565
						// extract rgb components
						r_textel  = ((textel >> 11)       ); 
						g_textel  = ((textel >> 5)  & 0x3f); 
						b_textel =   (textel        & 0x1f);
#else //PIXEL_FORMAT555
						r_textel  = ((textel >> 10) & 0x1f); 
						g_textel  = ((textel >> 5)  & 0x1f); 
						b_textel =   (textel        & 0x1f);
#endif
						// modulate textel with gouraud shading
						r_textel*=ui; 
						g_textel*=vi;
						b_textel*=wi;

						// finally write pixel, note that we did the math such that the results are r*32, g*64, b*32
						// hence we need to divide the results by 32,64,32 respetively, BUT since we need to shift
						// the results to fit into the destination 5.6.5 word, we can take advantage of the shifts
						// and they all cancel out for the most part, but we will need logical anding, we will do
						// it later when we optimize more...
#ifdef NM3D_PIXELFORMAT_565
						screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT+8)) + 
							((g_textel >> (NM3D_FIXP16_SHIFT+8)) << 5) + 
							((r_textel >> (NM3D_FIXP16_SHIFT+8)) << 11));
#else // PIXEL_FORMAT555
						screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT+8)) + 
							((g_textel >> (NM3D_FIXP16_SHIFT+8)) << 5) + 
							((r_textel >> (NM3D_FIXP16_SHIFT+8)) << 10));
#endif
						// interpolate u,v
						ui+=du;
						vi+=dv;
						wi+=dw;

						si+=ds;
						ti+=dt;

					} // end for xi

					// interpolate u,v,w,x along right and left edge
					xl+=dxdyl;
					ul+=dudyl;
					vl+=dvdyl;
					wl+=dwdyl;

					sl+=dsdyl;
					tl+=dtdyl;


					xr+=dxdyr;
					ur+=dudyr;
					vr+=dvdyr;
					wr+=dwdyr;

					sr+=dsdyr;
					tr+=dtdyr;


					// advance screen ptr
					screen_ptr+=mem_pitch;

				} // end for y

			} // end if non-clipped

		} // end if
		else
			if (tri_type==NM3D_TRI_TYPE_GENERAL)
			{

				// first test for bottom clip, always
				if ((yend = y2) > max_clip_y)
					yend = max_clip_y;

				// pre-test y clipping status
				if (y1 < min_clip_y)
				{
					// compute all deltas
					// LHS
					dyl = (y2 - y1);

					dxdyl = ((x2  - x1)  << NM3D_FIXP16_SHIFT)/dyl;
					dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT)/dyl;  
					dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT)/dyl;    
					dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT)/dyl;  


					dsdyl = ((ts2 - ts1) << NM3D_FIXP16_SHIFT)/dyl;    
					dtdyl = ((tt2 - tt1) << NM3D_FIXP16_SHIFT)/dyl;  

					// RHS
					dyr = (y2 - y0);	

					dxdyr = ((x2  - x0)  << NM3D_FIXP16_SHIFT)/dyr;
					dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT)/dyr;  
					dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT)/dyr;   
					dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT)/dyr;   

					dsdyr = ((ts2 - ts0) << NM3D_FIXP16_SHIFT)/dyr;   
					dtdyr = ((tt2 - tt0) << NM3D_FIXP16_SHIFT)/dyr;  

					// compute overclip
					dyr = (min_clip_y - y0);
					dyl = (min_clip_y - y1);

					// computer new LHS starting values
					xl = dxdyl*dyl + (x1  << NM3D_FIXP16_SHIFT);

					ul = dudyl*dyl + (tu1 << NM3D_FIXP16_SHIFT);
					vl = dvdyl*dyl + (tv1 << NM3D_FIXP16_SHIFT);
					wl = dwdyl*dyl + (tw1 << NM3D_FIXP16_SHIFT);

					sl = dsdyl*dyl + (ts1 << NM3D_FIXP16_SHIFT);
					tl = dtdyl*dyl + (tt1 << NM3D_FIXP16_SHIFT);


					// compute new RHS starting values
					xr = dxdyr*dyr + (x0  << NM3D_FIXP16_SHIFT);

					ur = dudyr*dyr + (tu0 << NM3D_FIXP16_SHIFT);
					vr = dvdyr*dyr + (tv0 << NM3D_FIXP16_SHIFT);
					wr = dwdyr*dyr + (tw0 << NM3D_FIXP16_SHIFT);

					sr = dsdyr*dyr + (ts0 << NM3D_FIXP16_SHIFT);
					tr = dtdyr*dyr + (tt0 << NM3D_FIXP16_SHIFT);


					// compute new starting y
					ystart = min_clip_y;

					// test if we need swap to keep rendering left to right
					if (dxdyr > dxdyl)
					{
						SWAP(dxdyl,dxdyr/*,temp*/);
						SWAP(dudyl,dudyr/*,temp*/);
						SWAP(dvdyl,dvdyr/*,temp*/);
						SWAP(dwdyl,dwdyr/*,temp*/);

						SWAP(dsdyl,dsdyr/*,temp*/);
						SWAP(dtdyl,dtdyr/*,temp*/);

						SWAP(xl,xr/*,temp*/);
						SWAP(ul,ur/*,temp*/);
						SWAP(vl,vr/*,temp*/);
						SWAP(wl,wr/*,temp*/);

						SWAP(sl,sr/*,temp*/);
						SWAP(tl,tr/*,temp*/);

						SWAP(x1,x2/*,temp*/);
						SWAP(y1,y2/*,temp*/);
						SWAP(tu1,tu2/*,temp*/);
						SWAP(tv1,tv2/*,temp*/);
						SWAP(tw1,tw2/*,temp*/);

						SWAP(ts1,ts2/*,temp*/);
						SWAP(tt1,tt2/*,temp*/);

						// set interpolation restart
						irestart = NM3D_INTERP_RHS;

					} // end if

				} // end if
				else
				{
					if (y0 < min_clip_y)
					{
						// compute all deltas
						// LHS
						dyl = (y1 - y0);

						dxdyl = ((x1  - x0)  << NM3D_FIXP16_SHIFT)/dyl;
						dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT)/dyl;  
						dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT)/dyl;    
						dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT)/dyl; 

						dsdyl = ((ts1 - ts0) << NM3D_FIXP16_SHIFT)/dyl;    
						dtdyl = ((tt1 - tt0) << NM3D_FIXP16_SHIFT)/dyl; 


						// RHS
						dyr = (y2 - y0);	

						dxdyr = ((x2  - x0)  << NM3D_FIXP16_SHIFT)/dyr;
						dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT)/dyr;  
						dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT)/dyr;   
						dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT)/dyr;   

						dsdyr = ((ts2 - ts0) << NM3D_FIXP16_SHIFT)/dyr;   
						dtdyr = ((tt2 - tt0) << NM3D_FIXP16_SHIFT)/dyr;   


						// compute overclip
						dy = (min_clip_y - y0);

						// computer new LHS starting values
						xl = dxdyl*dy + (x0  << NM3D_FIXP16_SHIFT);
						ul = dudyl*dy + (tu0 << NM3D_FIXP16_SHIFT);
						vl = dvdyl*dy + (tv0 << NM3D_FIXP16_SHIFT);
						wl = dwdyl*dy + (tw0 << NM3D_FIXP16_SHIFT);

						sl = dsdyl*dy + (ts0 << NM3D_FIXP16_SHIFT);
						tl = dtdyl*dy + (tt0 << NM3D_FIXP16_SHIFT);


						// compute new RHS starting values
						xr = dxdyr*dy + (x0  << NM3D_FIXP16_SHIFT);
						ur = dudyr*dy + (tu0 << NM3D_FIXP16_SHIFT);
						vr = dvdyr*dy + (tv0 << NM3D_FIXP16_SHIFT);
						wr = dwdyr*dy + (tw0 << NM3D_FIXP16_SHIFT);

						sr = dsdyr*dy + (ts0 << NM3D_FIXP16_SHIFT);
						tr = dtdyr*dy + (tt0 << NM3D_FIXP16_SHIFT);

						// compute new starting y
						ystart = min_clip_y;

						// test if we need swap to keep rendering left to right
						if (dxdyr < dxdyl)
						{
							SWAP(dxdyl,dxdyr/*,temp*/);
							SWAP(dudyl,dudyr/*,temp*/);
							SWAP(dvdyl,dvdyr/*,temp*/);
							SWAP(dwdyl,dwdyr/*,temp*/);

							SWAP(dsdyl,dsdyr/*,temp*/);
							SWAP(dtdyl,dtdyr/*,temp*/);

							SWAP(xl,xr/*,temp*/);
							SWAP(ul,ur/*,temp*/);
							SWAP(vl,vr/*,temp*/);
							SWAP(wl,wr/*,temp*/);

							SWAP(sl,sr/*,temp*/);
							SWAP(tl,tr/*,temp*/);


							SWAP(x1,x2/*,temp*/);
							SWAP(y1,y2/*,temp*/);
							SWAP(tu1,tu2/*,temp*/);
							SWAP(tv1,tv2/*,temp*/);
							SWAP(tw1,tw2/*,temp*/);

							SWAP(ts1,ts2/*,temp*/);
							SWAP(tt1,tt2/*,temp*/);

							// set interpolation restart
							irestart = NM3D_INTERP_RHS;

						} // end if

					} // end if
					else
					{
						// no initial y clipping

						// compute all deltas
						// LHS
						dyl = (y1 - y0);

						dxdyl = ((x1  - x0)  << NM3D_FIXP16_SHIFT)/dyl;
						dudyl = ((tu1 - tu0) << NM3D_FIXP16_SHIFT)/dyl;  
						dvdyl = ((tv1 - tv0) << NM3D_FIXP16_SHIFT)/dyl;    
						dwdyl = ((tw1 - tw0) << NM3D_FIXP16_SHIFT)/dyl;   

						dsdyl = ((ts1 - ts0) << NM3D_FIXP16_SHIFT)/dyl;    
						dtdyl = ((tt1 - tt0) << NM3D_FIXP16_SHIFT)/dyl;   


						// RHS
						dyr = (y2 - y0);	

						dxdyr = ((x2 - x0)   << NM3D_FIXP16_SHIFT)/dyr;
						dudyr = ((tu2 - tu0) << NM3D_FIXP16_SHIFT)/dyr;  
						dvdyr = ((tv2 - tv0) << NM3D_FIXP16_SHIFT)/dyr;   		
						dwdyr = ((tw2 - tw0) << NM3D_FIXP16_SHIFT)/dyr;

						dsdyr = ((ts2 - ts0) << NM3D_FIXP16_SHIFT)/dyr;   		
						dtdyr = ((tt2 - tt0) << NM3D_FIXP16_SHIFT)/dyr;


						// no clipping y

						// set starting values
						xl = (x0 << NM3D_FIXP16_SHIFT);
						xr = (x0 << NM3D_FIXP16_SHIFT);

						ul = (tu0 << NM3D_FIXP16_SHIFT);
						vl = (tv0 << NM3D_FIXP16_SHIFT);
						wl = (tw0 << NM3D_FIXP16_SHIFT);

						sl = (ts0 << NM3D_FIXP16_SHIFT);
						tl = (tt0 << NM3D_FIXP16_SHIFT);

						ur = (tu0 << NM3D_FIXP16_SHIFT);
						vr = (tv0 << NM3D_FIXP16_SHIFT);
						wr = (tw0 << NM3D_FIXP16_SHIFT);

						sr = (ts0 << NM3D_FIXP16_SHIFT);
						tr = (tt0 << NM3D_FIXP16_SHIFT);

						// set starting y
						ystart = y0;

						// test if we need swap to keep rendering left to right
						if (dxdyr < dxdyl)
						{
							SWAP(dxdyl,dxdyr/*,temp*/);
							SWAP(dudyl,dudyr/*,temp*/);
							SWAP(dvdyl,dvdyr/*,temp*/);
							SWAP(dwdyl,dwdyr/*,temp*/);

							SWAP(dsdyl,dsdyr/*,temp*/);
							SWAP(dtdyl,dtdyr/*,temp*/);


							SWAP(xl,xr/*,temp*/);
							SWAP(ul,ur/*,temp*/);
							SWAP(vl,vr/*,temp*/);
							SWAP(wl,wr/*,temp*/);

							SWAP(sl,sr/*,temp*/);
							SWAP(tl,tr/*,temp*/);


							SWAP(x1,x2/*,temp*/);
							SWAP(y1,y2/*,temp*/);
							SWAP(tu1,tu2/*,temp*/);
							SWAP(tv1,tv2/*,temp*/);
							SWAP(tw1,tw2/*,temp*/);


							SWAP(ts1,ts2/*,temp*/);
							SWAP(tt1,tt2/*,temp*/);

							// set interpolation restart
							irestart = NM3D_INTERP_RHS;

						} // end if

					} // end else
				}

				// test for horizontal clipping
				if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
					(x1 < min_clip_x) || (x1 > max_clip_x) ||
					(x2 < min_clip_x) || (x2 > max_clip_x))
				{
					// clip version
					// x clipping

					// point screen ptr to starting line
					screen_ptr = dest_buffer + (ystart * mem_pitch);

					for (yi = ystart; yi < yend; yi++)
					{
						// compute span endpoints
						xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
						xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

						// compute starting points for u,v,w interpolants
						ui = ul + NM3D_FIXP16_ROUND_UP;
						vi = vl + NM3D_FIXP16_ROUND_UP;
						wi = wl + NM3D_FIXP16_ROUND_UP;

						si = sl + NM3D_FIXP16_ROUND_UP;
						ti = tl + NM3D_FIXP16_ROUND_UP;

						// compute u,v interpolants
						if ((dx = (xend - xstart)) > 0)
						{
							du = (ur - ul) / dx;
							dv = (vr - vl) / dx;
							dw = (wr - wl) / dx;

							ds = (sr - sl) / dx;
							dt = (tr - tl) / dx;

						} // end if
						else
						{
							du = (ur - ul);
							dv = (vr - vl);
							dw = (wr - wl);

							ds = (sr - sl);
							dt = (tr - tl);

						} // end else

						///////////////////////////////////////////////////////////////////////

						// test for x clipping, LHS
						if (xstart < min_clip_x)
						{
							// compute x overlap
							dx = min_clip_x - xstart;

							// slide interpolants over
							ui += dx * du;
							vi += dx * dv;
							wi += dx * dw;

							si += dx * ds;
							ti += dx * dt;

							// set x to left clip edge
							xstart = min_clip_x;

						} // end if

						// test for x clipping RHS
						if (xend > max_clip_x)
							xend = max_clip_x;

						///////////////////////////////////////////////////////////////////////

						// draw span
						for (xi = xstart; xi < xend; xi++)
						{
							// write textel assume 5.6.5
							// screen_ptr[xi] = ( (ui >> (FIXP16_SHIFT+3)) << 11) +
							//                 ( (vi >> (FIXP16_SHIFT+2)) << 5) +
							//                   (wi >> (FIXP16_SHIFT+3) );

							// get textel first
							textel = textmap[(si >> NM3D_FIXP16_SHIFT) + ((ti >> NM3D_FIXP16_SHIFT) << texture_shift2)];
#ifdef NM3D_PIXELFORMAT_565
							// extract rgb components
							r_textel = ((textel >> 11));
							g_textel = ((textel >> 5) & 0x3f);
							b_textel = (textel & 0x1f);
#else // PIXEL_FORMAT555
							r_textel = ((textel >> 10) & 0x1f);
							g_textel = ((textel >> 5) & 0x1f);
							b_textel = (textel & 0x1f);
#endif
							// modulate textel with gouraud shading
							r_textel *= ui;
							g_textel *= vi;
							b_textel *= wi;

							// finally write pixel, note that we did the math such that the results are r*32, g*64, b*32
							// hence we need to divide the results by 32,64,32 respetively, BUT since we need to shift
							// the results to fit into the destination 5.6.5 word, we can take advantage of the shifts
							// and they all cancel out for the most part, but we will need logical anding, we will do
							// it later when we optimize more...
#ifdef NM3D_PIXELFORMAT_565
							screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT + 8)) +
											  ((g_textel >> (NM3D_FIXP16_SHIFT + 8)) << 5) +
											  ((r_textel >> (NM3D_FIXP16_SHIFT + 8)) << 11));
#else // PIXEL_FORMAT555
							screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT + 8)) +
											  ((g_textel >> (NM3D_FIXP16_SHIFT + 8)) << 5) +
											  ((r_textel >> (NM3D_FIXP16_SHIFT + 8)) << 10));
#endif
							// interpolate u,v
							ui += du;
							vi += dv;
							wi += dw;

							si += ds;
							ti += dt;

						} // end for xi

						// interpolate u,v,w,x along right and left edge
						xl += dxdyl;
						ul += dudyl;
						vl += dvdyl;
						wl += dwdyl;

						sl += dsdyl;
						tl += dtdyl;

						xr += dxdyr;
						ur += dudyr;
						vr += dvdyr;
						wr += dwdyr;

						sr += dsdyr;
						tr += dtdyr;

						// advance screen ptr
						screen_ptr += mem_pitch;

						// test for yi hitting second region, if so change interpolant
						if (yi == yrestart)
						{
							// test interpolation side change flag

							if (irestart == NM3D_INTERP_LHS)
							{
								// LHS
								dyl = (y2 - y1);

								dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;
								dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dyl;
								dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dyl;
								dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dyl;

								dsdyl = ((ts2 - ts1) << NM3D_FIXP16_SHIFT) / dyl;
								dtdyl = ((tt2 - tt1) << NM3D_FIXP16_SHIFT) / dyl;

								// set starting values
								xl = (x1 << NM3D_FIXP16_SHIFT);
								ul = (tu1 << NM3D_FIXP16_SHIFT);
								vl = (tv1 << NM3D_FIXP16_SHIFT);
								wl = (tw1 << NM3D_FIXP16_SHIFT);

								sl = (ts1 << NM3D_FIXP16_SHIFT);
								tl = (tt1 << NM3D_FIXP16_SHIFT);

								// interpolate down on LHS to even up
								xl += dxdyl;
								ul += dudyl;
								vl += dvdyl;
								wl += dwdyl;

								sl += dsdyl;
								tl += dtdyl;

							} // end if
							else
							{
								// RHS
								dyr = (y1 - y2);

								dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;
								dudyr = ((tu1 - tu2) << NM3D_FIXP16_SHIFT) / dyr;
								dvdyr = ((tv1 - tv2) << NM3D_FIXP16_SHIFT) / dyr;
								dwdyr = ((tw1 - tw2) << NM3D_FIXP16_SHIFT) / dyr;

								dsdyr = ((ts1 - ts2) << NM3D_FIXP16_SHIFT) / dyr;
								dtdyr = ((tt1 - tt2) << NM3D_FIXP16_SHIFT) / dyr;

								// set starting values
								xr = (x2 << NM3D_FIXP16_SHIFT);
								ur = (tu2 << NM3D_FIXP16_SHIFT);
								vr = (tv2 << NM3D_FIXP16_SHIFT);
								wr = (tw2 << NM3D_FIXP16_SHIFT);

								sr = (ts2 << NM3D_FIXP16_SHIFT);
								tr = (tt2 << NM3D_FIXP16_SHIFT);

								// interpolate down on RHS to even up
								xr += dxdyr;
								ur += dudyr;
								vr += dvdyr;
								wr += dwdyr;

								sr += dsdyr;
								tr += dtdyr;

							} // end else

						} // end if

					} // end for y

				} // end if
				else
				{
					// no x clipping
					// point screen ptr to starting line
					screen_ptr = dest_buffer + (ystart * mem_pitch);

					for (yi = ystart; yi < yend; yi++)
					{
						// compute span endpoints
						xstart = ((xl + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);
						xend = ((xr + NM3D_FIXP16_ROUND_UP) >> NM3D_FIXP16_SHIFT);

						// compute starting points for u,v,w interpolants
						ui = ul + NM3D_FIXP16_ROUND_UP;
						vi = vl + NM3D_FIXP16_ROUND_UP;
						wi = wl + NM3D_FIXP16_ROUND_UP;

						si = sl + NM3D_FIXP16_ROUND_UP;
						ti = tl + NM3D_FIXP16_ROUND_UP;

						// compute u,v interpolants
						if ((dx = (xend - xstart)) > 0)
						{
							du = (ur - ul) / dx;
							dv = (vr - vl) / dx;
							dw = (wr - wl) / dx;

							ds = (sr - sl) / dx;
							dt = (tr - tl) / dx;

						} // end if
						else
						{
							du = (ur - ul);
							dv = (vr - vl);
							dw = (wr - wl);

							ds = (sr - sl);
							dt = (tr - tl);

						} // end else

						// draw span
						for (xi = xstart; xi < xend; xi++)
						{
							// write textel assume 5.6.5
							// screen_ptr[xi] = ( (ui >> (FIXP16_SHIFT+3)) << 11) +
							//                 ( (vi >> (FIXP16_SHIFT+2)) << 5) +
							//                   (wi >> (FIXP16_SHIFT+3) );

							// get textel first
							textel = textmap[(si >> NM3D_FIXP16_SHIFT) + ((ti >> NM3D_FIXP16_SHIFT) << texture_shift2)];
#ifdef NM3D_PIXELFORMAT_565
							// extract rgb components
							r_textel = ((textel >> 11));
							g_textel = ((textel >> 5) & 0x3f);
							b_textel = (textel & 0x1f);
#else // PIXEL_FORMAT555
							r_textel = ((textel >> 10) & 0x1f);
							g_textel = ((textel >> 5) & 0x1f);
							b_textel = (textel & 0x1f);
#endif
							// modulate textel with gouraud shading
							r_textel *= ui;
							g_textel *= vi;
							b_textel *= wi;

							// finally write pixel, note that we did the math such that the results are r*32, g*64, b*32
							// hence we need to divide the results by 32,64,32 respetively, BUT since we need to shift
							// the results to fit into the destination 5.6.5 word, we can take advantage of the shifts
							// and they all cancel out for the most part, but we will need logical anding, we will do
							// it later when we optimize more...
#ifdef NM3D_PIXELFORMAT_565
							screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT + 8)) +
											  ((g_textel >> (NM3D_FIXP16_SHIFT + 8)) << 5) +
											  ((r_textel >> (NM3D_FIXP16_SHIFT + 8)) << 11));
#else
							screen_ptr[xi] = ((b_textel >> (NM3D_FIXP16_SHIFT + 8)) +
											  ((g_textel >> (NM3D_FIXP16_SHIFT + 8)) << 5) +
											  ((r_textel >> (NM3D_FIXP16_SHIFT + 8)) << 10));
#endif
							// interpolate u,v
							ui += du;
							vi += dv;
							wi += dw;

							si += ds;
							ti += dt;

						} // end for xi

						// interpolate u,v,w,x along right and left edge
						xl += dxdyl;
						ul += dudyl;
						vl += dvdyl;
						wl += dwdyl;

						sl += dsdyl;
						tl += dtdyl;

						xr += dxdyr;
						ur += dudyr;
						vr += dvdyr;
						wr += dwdyr;

						sr += dsdyr;
						tr += dtdyr;

						// advance screen ptr
						screen_ptr += mem_pitch;

						// test for yi hitting second region, if so change interpolant
						if (yi == yrestart)
						{
							// test interpolation side change flag

							if (irestart == NM3D_INTERP_LHS)
							{
								// LHS
								dyl = (y2 - y1);

								dxdyl = ((x2 - x1) << NM3D_FIXP16_SHIFT) / dyl;
								dudyl = ((tu2 - tu1) << NM3D_FIXP16_SHIFT) / dyl;
								dvdyl = ((tv2 - tv1) << NM3D_FIXP16_SHIFT) / dyl;
								dwdyl = ((tw2 - tw1) << NM3D_FIXP16_SHIFT) / dyl;

								dsdyl = ((ts2 - ts1) << NM3D_FIXP16_SHIFT) / dyl;
								dtdyl = ((tt2 - tt1) << NM3D_FIXP16_SHIFT) / dyl;

								// set starting values
								xl = (x1 << NM3D_FIXP16_SHIFT);
								ul = (tu1 << NM3D_FIXP16_SHIFT);
								vl = (tv1 << NM3D_FIXP16_SHIFT);
								wl = (tw1 << NM3D_FIXP16_SHIFT);

								sl = (ts1 << NM3D_FIXP16_SHIFT);
								tl = (tt1 << NM3D_FIXP16_SHIFT);

								// interpolate down on LHS to even up
								xl += dxdyl;
								ul += dudyl;
								vl += dvdyl;
								wl += dwdyl;

								sl += dsdyl;
								tl += dtdyl;

							} // end if
							else
							{
								// RHS
								dyr = (y1 - y2);

								dxdyr = ((x1 - x2) << NM3D_FIXP16_SHIFT) / dyr;
								dudyr = ((tu1 - tu2) << NM3D_FIXP16_SHIFT) / dyr;
								dvdyr = ((tv1 - tv2) << NM3D_FIXP16_SHIFT) / dyr;
								dwdyr = ((tw1 - tw2) << NM3D_FIXP16_SHIFT) / dyr;

								dsdyr = ((ts1 - ts2) << NM3D_FIXP16_SHIFT) / dyr;
								dtdyr = ((tt1 - tt2) << NM3D_FIXP16_SHIFT) / dyr;

								// set starting values
								xr = (x2 << NM3D_FIXP16_SHIFT);
								ur = (tu2 << NM3D_FIXP16_SHIFT);
								vr = (tv2 << NM3D_FIXP16_SHIFT);
								wr = (tw2 << NM3D_FIXP16_SHIFT);

								sr = (ts2 << NM3D_FIXP16_SHIFT);
								tr = (tt2 << NM3D_FIXP16_SHIFT);

								// interpolate down on RHS to even up
								xr += dxdyr;
								ur += dudyr;
								vr += dvdyr;
								wr += dwdyr;

								sr += dsdyr;
								tr += dtdyr;

							} // end else

						} // end if

					} // end for y

				} // end else

			} // end if

} // end Draw_Textured_TriangleGS_16
#endif
//////////////////////////////////////////////////////////////////////////////