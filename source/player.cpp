// player.cpp: implementation of the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "player.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CPlayer::CPlayer()
	{
		Init();
	}

	void CPlayer::Init()
	{
		m_nNumPictures = 0;
		for(int i=0; i<NM3D_MAX_PICTURE_COUNT; i++)
			m_pPictureList[i] = NULL;
	}
	
	int CPlayer::AddPicture(CPicture* pic)
	{
		int nNumScene = m_nNumPictures;
		if( nNumScene == NM3D_MAX_PICTURE_COUNT || pic == NULL)
			return -1;
		else
		{
			m_pPictureList[nNumScene] = pic;
			m_nNumPictures++;
		}

		return m_nNumPictures;
	}

	int CPlayer::InsertPicture(int n, CPicture* pic)
	{
		int nNumScene = m_nNumPictures;
		if( nNumScene == NM3D_MAX_PICTURE_COUNT || pic == NULL)
			return -1;
		for(int i=nNumScene; i>=n; i--)
			m_pPictureList[i] = m_pPictureList[i-1];

		m_pPictureList[n] = pic;
		m_nNumPictures++;
		return m_nNumPictures;
	}

	int CPlayer::GetNumPictures()
	{
		return m_nNumPictures;
	}

	void CPlayer::ShowPicture(int n)
	{
		if(n < 0)//if n<0 then draw all picture
		{
			for(int i=0; i<m_nNumPictures; i++)
			{
				if(m_pPictureList[i])
					m_pPictureList[i]->Draw(i);
			}
		}
		else if(n<m_nNumPictures) //draw the specific picture
		{
			m_pPictureList[n]->Draw(n);
		}
	}
}
