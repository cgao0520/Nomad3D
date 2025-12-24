// image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "image.h"
#include "fileaccess.h"
#include "log.h"
#include <cassert>
#include <cstring>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CImage::CImage()
	{
		m_nHeight = 0;
		m_nWidth = 0;
		m_ucpData = NULL;
	}
	
	CImage::~CImage()
	{
		if(m_ucpData)
			delete[] m_ucpData;
	}

	const char* CImage::GetFileName(const char *path)
	{
		assert(path);

		const char *last_slash = strrchr(path, '/');	  // Linux separator
		const char *last_backslash = strrchr(path, '\\'); // Windows separator
		const char *final_ptr = NULL;

		// Pick the one that appears furthest to the right (largest memory address)
		if (last_slash > last_backslash)
		{
			final_ptr = last_slash;
		}
		else
		{
			final_ptr = last_backslash;
		}

		// If no separator was found, return the original path (it's just a filename)
		if (final_ptr == NULL)
		{
			return path;
		}

		// Return the position right after the separator
		return final_ptr + 1;
	}

	//virtual ~CImage();
	bool CImage::LoadTexture(const char* szTextureFileName)
	{
		if(!szTextureFileName)
			return false;

		const char* fileName = GetFileName(szTextureFileName);
		if (!fileName)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Cannot open %s as processed filename is empty", szTextureFileName);
			return false;
		}

		// Currently only support BMP files with ID: 42 4D, and 24bit/32bit without pallete
		CFileAccess texBuff(fileName);
		unsigned char* ucpBuffer = texBuff.GetBuffer();
		if(!ucpBuffer)
			return false;

		unsigned char* ucpPtr = ucpBuffer;

		SBitMapFileHeader* pBMPHeader = (SBitMapFileHeader*)ucpPtr;//SBitMapFileHeader bmpheader = *pBMPHeader;

		//only deal with this type of BMP file
		if( !(pBMPHeader->ucType[0]==0x42 && pBMPHeader->ucType[1]==0x4d) )
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Not supported FLAG file: %s", fileName);
			return false;
		}
		
		ucpPtr += sizeof(SBitMapFileHeader);

		SBitMapInfoHeader* pBitMapInfoHeader = (SBitMapInfoHeader*)ucpPtr;//SBitMapInfoHeader bmpheaderinfo = *pBitMapInfoHeader;
		int nWidth = pBitMapInfoHeader->unWidth;
		int nHeight = pBitMapInfoHeader->nHeight;
		bool bInverse = nHeight > 0 ? true : false;
		int nBitCount = pBitMapInfoHeader->usBitCount;

		//Only support 24bit and 32bit color
		if(nBitCount != 24 && nBitCount != 32)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "It's not a 24bit or 32bit file: %s", fileName);
			return false;
		}

		//No Palette
		if(pBitMapInfoHeader->unClrUsed != 0 || pBitMapInfoHeader->unClrImportant != 0)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "This version of Nomad3D is not support palette: %s", fileName);
			return false;
		}

		ucpPtr += sizeof(SBitMapInfoHeader);

		//////////////////////////////////////////////////////////////////////////

		m_nWidth = nWidth;
		m_nHeight = nHeight > 0 ? nHeight : -nHeight;
		int nTotlePixels = m_nHeight*m_nWidth;
		m_nBytesPerLine = m_nWidth*sizeof(CRGB);
		m_ucpData = new CRGB[nTotlePixels];
		
		int nBytePerPixelInBMPFile = nBitCount / 8; // Number of bytes each pixel occupies in BMP file
		
		int nCount = 0;
		while(nCount < nTotlePixels)
		{
			m_ucpData[nCount] = CRGB(/*R*/ucpPtr[2],/*G*/ucpPtr[1],/*B*/ucpPtr[0]);
			nCount++;
			ucpPtr += nBytePerPixelInBMPFile;
		}
		
		if(bInverse)
			FlipBitmap();
/*
		//////////////////////////////////////////////////////////////////////////
		bmpheader.unSize = 14 + 40 + nTotlePixels*sizeof(CRGB);
		bmpheaderinfo.usBitCount = 16;
		bmpheaderinfo.nHeight = bmpheaderinfo.nHeight>0?-bmpheaderinfo.nHeight:bmpheaderinfo.nHeight;
		bmpheaderinfo.unSizeImage = nTotlePixels*sizeof(CRGB);
		char ffname[500];
		memset(ffname,0,500);
		sprintf(ffname,"d:\\%s_new.bmp",szTextureFileName);
		FILE* fp = fopen(ffname,"wb");
		fwrite(&bmpheader,14,1,fp);
		fwrite(&bmpheaderinfo,40,1,fp);
		fwrite(m_ucpData,sizeof(CRGB)*nTotlePixels,1,fp);
		fclose(fp);
		//////////////////////////////////////////////////////////////////////////
*/
		return true;
	}

	bool CImage::FlipBitmap()
	{
		unsigned char* pLine = new unsigned char[m_nBytesPerLine];
		if(!pLine)
			return false;
		
		unsigned char* pData = GetData();
		int n = m_nHeight/2;
		for(int i=0; i<n; i++)
		{
			int j = m_nHeight-i-1;
			memcpy(pLine, pData+j*m_nBytesPerLine, m_nBytesPerLine);
			memcpy(pData+j*m_nBytesPerLine, pData+i*m_nBytesPerLine, m_nBytesPerLine);
			memcpy(pData+i*m_nBytesPerLine, pLine, m_nBytesPerLine);
		}

		delete[] pLine;
		return true;
	}

}
