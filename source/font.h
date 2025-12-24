// Font.h: interface for the CFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FONT_H__9DA912C9_F93E_4ED9_BD51_672A30C1BDE6__INCLUDED_)
#define _FONT_H__9DA912C9_F93E_4ED9_BD51_672A30C1BDE6__INCLUDED_

#include "rgb.h"

#define FONT8_ASC_COUNT			95	// Number of characters
#define FONT8_ASC_START			32	// First character (ASCII code)
#define FONT8_ASC_WIDTH			8	// Width (pixels) of character
#define FONT8_ASC_HEIGHT		8	// Height (pixels) of character

#define ROW_SPACE				1	// Line/row space (pixels)
#define COL_SPACE				1	// Column space (pixels)
#define TAB_SPACE_NUM			4	// Nubmer of spaces for Tab

namespace Nomad3D
{
	enum EFontType
	{
		enFont8
	};

	struct STextPos
	{
		int row;
		int col;
	} ;

	class CFont
	{
	public:
		CFont(unsigned char* pBuff)
		{
			m_enCurFontType = enFont8;
			m_CurTextPos.row = m_CurTextPos.col = 0;
			m_pBuff = pBuff;
			m_rgbText = CRGBA(255,255,255,255);
		};
		virtual ~CFont(){};
		//////////////////////////////////////////////////////////////////////////
		virtual inline int DrawChar(int nRow, int nCol, int nIndex, CRGBA color,  unsigned short* buffer) = 0;
		virtual STextPos DrawString(int row,int col,const char* p, NM3D_BUFFER buffer)=0;
		inline void ResetPos()
		{
			m_CurTextPos.row = m_CurTextPos.col = 0;
		}
		inline void ResetPos(int row, int col)
		{
			m_CurTextPos.row = row;
			m_CurTextPos.col = col;
		}
		inline void SetCurFont(EFontType enFT=enFont8)
		{
			m_enCurFontType = enFT;
		}
		EFontType GetCurFont()
		{
			return m_enCurFontType;
		}
		CRGBA SetTextColor(CRGBA rgba)
		{
			CRGBA t = m_rgbText;
			m_rgbText = rgba;
			return t;
		}
	protected:
		EFontType	m_enCurFontType;
		STextPos	m_CurTextPos;
		unsigned char*	m_pBuff;
		CRGBA		m_rgbText;
	};
}
#endif // !defined(_FONT_H__9DA912C9_F93E_4ED9_BD51_672A30C1BDE6__INCLUDED_)
