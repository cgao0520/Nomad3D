#ifndef VIEWINFO_H_6013254D_D806_4931_93A4_5C9C025C5CB5
#define VIEWINFO_H_6013254D_D806_4931_93A4_5C9C025C5CB5

#include "config.h"
#include "platform.h"
#include "constant.h"
#include "clipbox.h"

namespace Nomad3D
{
	class CViewInfo
	{
		friend class CPicture;
	public:
		inline CViewInfo()
		{
			m_nStartX = 0;
			m_nStartY = 0;
			m_nWidth = NM3D_WINDOW_WIDTH;
			m_nHeight = NM3D_WINDOW_HEIGHT;
		}
		inline ~CViewInfo()
		{

		}
		inline CViewInfo(const CViewInfo& vi)
		{
			Assign(vi.m_nStartX, vi.m_nStartY, vi.m_nWidth, vi.m_nHeight, vi.m_rgbClearColor);
		}
		inline CViewInfo(int nStartX, int nStartY, int nWidth, int nHeight, CRGBA rgbClearColor)
		{
			Assign(nStartX, nStartY, nWidth, nHeight, rgbClearColor);
		}
		inline CViewInfo& operator = (const CViewInfo& vi)
		{
			Assign(vi.m_nStartX, vi.m_nStartY, vi.m_nWidth, vi.m_nHeight, vi.m_rgbClearColor);
			return *this;
		}
		inline void Assign(int nStartX, int nStartY, int nWidth, int nHeight, CRGBA rgbClearColor)
		{
			m_nStartX = nStartX;
			m_nStartY = nStartY;
			m_nWidth = nWidth;
			m_nHeight = nHeight;

			m_rgbClearColor=rgbClearColor;
		}
		
		CClipBox GetClipBox()
		{
			CClipBox cb(
				m_nStartX, 
				m_nStartX+m_nWidth-1,
				m_nStartY,
				m_nStartY+m_nHeight-1
				);
			return cb;
		}

		void SlideX(int nXOffset)
		{
			m_nStartX += nXOffset;
		}
		void SlideY(int nYOffset)
		{
			m_nStartY += nYOffset;
		}
		void Move(int nXOffset, int nYOffset)
		{
			m_nStartX += nXOffset;
			m_nStartY += nYOffset;
		}
		void MoveTo(int nStartX, int nStartY)
		{
			m_nStartX = nStartX;
			m_nStartY = nStartY;
		}
		//////////////////////////////////////////////////////////////////////////
		CViewInfo GetSlideX(int nXOffset)
		{
			CViewInfo vi(m_nStartX+nXOffset, m_nStartY, m_nWidth, m_nHeight, m_rgbClearColor);
			return vi;
		}
		CViewInfo GetSlideY(int nYOffset)
		{
			CViewInfo vi(m_nStartX, m_nStartY+nYOffset, m_nWidth, m_nHeight, m_rgbClearColor);
			return vi;
		}
		CViewInfo GetMove(int nXOffset, int nYOffset)
		{
			CViewInfo vi(m_nStartX+nXOffset, m_nStartY+nYOffset, m_nWidth, m_nHeight, m_rgbClearColor);
			return vi;
		}
		CViewInfo GetMoveTo(int nStartX, int nStartY)
		{
			CViewInfo vi(nStartX, nStartX, m_nWidth, m_nHeight, m_rgbClearColor);
			return vi;
		}
		inline CRGBA GetClearColor()
		{
			return m_rgbClearColor;
		}
		inline int GetViewWidth()
		{
			return m_nWidth;
		}
		inline int GetViewHeight()
		{
			return m_nHeight;
		}
		inline int GetStartX()
		{
			return m_nStartX;
		}
		inline int GetStartY()
		{
			return m_nStartY;
		}
	private:
		int m_nStartX;
		int m_nStartY;
		int m_nWidth;
		int m_nHeight;

		CRGBA m_rgbClearColor;
	};
}

#endif //VIEWINFO_H_6013254D_D806_4931_93A4_5C9C025C5CB5