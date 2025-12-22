// fileaccess.h: interface for the CFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEACCESS_H__49A2A0C0_06CF_400E_9BC7_9D807D04CA39__INCLUDED_)
#define AFX_FILEACCESS_H__49A2A0C0_06CF_400E_9BC7_9D807D04CA39__INCLUDED_

#include <stdio.h>

namespace Nomad3D
{
	class CFileAccess  
	{
	public:
		CFileAccess(const char* szFilename);
		virtual ~CFileAccess();
		unsigned char* GetBuffer();
		void ReleaseBuffer();
	protected:
		unsigned char* CreateBuffer(const char* szFilename);
	protected:
		unsigned char* m_pBuffer;
	};
}


#endif // !defined(AFX_FILEACCESS_H__49A2A0C0_06CF_400E_9BC7_9D807D04CA39__INCLUDED_)
