// image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__6B87BB81_C619_4ECE_8322_7B7BCFBBBA10__INCLUDED_)
#define AFX_IMAGE_H__6B87BB81_C619_4ECE_8322_7B7BCFBBBA10__INCLUDED_

#include "rgb.h"

namespace Nomad3D
{
#pragma pack(push, packing)
#pragma pack(1)
	typedef struct tagBitMapFileHeader 
	{
		unsigned char	ucType[2];
		unsigned int	unSize;
		unsigned short	usReserved1;
		unsigned short	usReserved2;
		unsigned int	unOffBits;
	} SBitMapFileHeader;

	typedef struct tagBitMapInfoHeader
	{
		unsigned int	unSize;
		unsigned int	unWidth;
		int				nHeight;
		unsigned short	usPlanes;
		unsigned short	usBitCount;
		unsigned int	unCompression;
		unsigned int	unSizeImage;
		unsigned int	unXPelsPerMeter;
		unsigned int	unYPelsPerMeter;
		unsigned int	unClrUsed;
		unsigned int	unClrImportant;
	} SBitMapInfoHeader;
	
	typedef struct tagPaletteEntry
	{
		unsigned char	ucRed;
		unsigned char	ucGreen;
		unsigned char	ucBlue;
		unsigned char	ucFlags;
	} SPaletteEntry;
#pragma pack(pop, packing)
/*	
	typedef struct _OFSTRUCT 
	{
		unsigned char cunsigned chars;
		unsigned char fFixedDisk;
		unsigned short nErrCode;
		unsigned short Reserved1;
		unsigned short Reserved2;
		CHAR szPathName[OFS_MAXPATHNAME];
	} OFSTRUCT, *LPOFSTRUCT, *POFSTRUCT;
*/

#pragma pack(push, packing)
#pragma pack(1)

	class CImage  
	{
	public:
		CImage();
		virtual ~CImage();
	public:
		inline int GetHeight() const {return m_nHeight;}
		inline int GetWidth() const {return m_nWidth;}
		inline int GetBytesPerLine() const {return m_nBytesPerLine;}
		inline CRGB* operator [] (int nIndex)
		{
			//assert(nIndex < m_nHeight*m_nWidth);
			return m_ucpData+nIndex;
		}
		inline const CRGB* operator [] (int nIndex) const
		{
			//assert(nIndex < m_nHeight*m_nWidth);
			return m_ucpData+nIndex;
		}
		
		inline const unsigned char* GetData() const 
		{
			return (const unsigned char*)m_ucpData;
		}
		inline unsigned char* GetData() 
		{
			return (unsigned char*)m_ucpData;
		}

		bool LoadTexture(const char* szTextureFileName);
		bool FlipBitmap();
	private:
		const char* GetFileName(const char* path);
	protected:
		int				m_nHeight;
		int				m_nWidth;
		int				m_nBytesPerLine;
		CRGB*			m_ucpData;
	};
	
#pragma pack(pop, packing)
}

#endif // !defined(AFX_IMAGE_H__6B87BB81_C619_4ECE_8322_7B7BCFBBBA10__INCLUDED_)
