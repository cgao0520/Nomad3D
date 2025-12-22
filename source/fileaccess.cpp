// fileaccess.cpp: implementation of the CFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "fileaccess.h"
#include "log.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{

	CFileAccess::CFileAccess(const char* szFilename)
	{
		m_pBuffer = NULL;
		CreateBuffer(szFilename);
	}
	
	CFileAccess::~CFileAccess()
	{
		ReleaseBuffer();
	}

	unsigned char* CFileAccess::GetBuffer()
	{
		return m_pBuffer;
	}

	unsigned char* CFileAccess::CreateBuffer(const char* szFilename)
	{
		ReleaseBuffer();
			
		//Open the file
		FILE * fp = fopen(szFilename, "rb");
		if(!fp)
		{
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Could not open %s", szFilename);
			return NULL;
		}
		
		//Get file size
		fseek(fp, 0, SEEK_END);
		int iEnd = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int iStart = ftell(fp);
		unsigned int uiSize = iEnd - iStart;
		//Allocate memory
		m_pBuffer = new unsigned char[uiSize];
		//ucpPtr = ucpBuffer;
		if(!m_pBuffer)
		{
			fclose(fp);
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Could not allocate memory");
			return NULL;
		}
		//Read file into buffer
		if(fread(m_pBuffer, 1, uiSize, fp) != uiSize)
		{
			fclose(fp);
			NM3D_DEBUG_PRINT(NM3D_DEBUG_COLOR_RED, "Could not read %s", szFilename);
			ReleaseBuffer();
			return NULL;
		}
		fclose(fp);
		
		return m_pBuffer;
		
	}

	void CFileAccess::ReleaseBuffer()
	{
		if(m_pBuffer)
			delete[] m_pBuffer;

		m_pBuffer = NULL;
	}

}
