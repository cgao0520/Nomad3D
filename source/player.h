// player.h: interface for the CPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PLAYER_H_F6BFC78F_F3A0_42DA_BC31_FBCA72D4BF33)
#define PLAYER_H_F6BFC78F_F3A0_42DA_BC31_FBCA72D4BF33

#include "picture.h"

namespace Nomad3D
{
	class CPlayer  
	{
	public:
		CPlayer();
		void Init();

	public:
		int AddPicture(CPicture* pic);
		int InsertPicture(int n, CPicture* pic);
		int GetNumPictures();
		void ShowPicture(int n);

	protected:
		int			m_nNumPictures;
		CPicture*	m_pPictureList[NM3D_MAX_PICTURE_COUNT]; // Picture at index 0 is the main picture

	};
}

#endif // !defined(PLAYER_H_F6BFC78F_F3A0_42DA_BC31_FBCA72D4BF33)
