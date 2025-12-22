// joint.h: interface for the CJoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(JOINT_H_BC0F3116_FED0_45A7_B9D5_2179FF91C660)
#define JOINT_H_BC0F3116_FED0_45A7_B9D5_2179FF91C660


#include "matrix4.h"

namespace Nomad3D
{
	class CKeyFrame
	{
	public:
		float m_fTime;
		float m_fParam[3];
	};

	class CJoint  
	{
	public:
		CJoint();
		virtual ~CJoint();
		void Assign(unsigned short nParentBoneIndex, const CMatrix4* pLocalMat, unsigned short usNumRotFrames, \
			unsigned short usNumTransFrames, const CKeyFrame* pRotKeyFrameData, const CKeyFrame* pTransKeyFrameData);
	public:
// 		char			m_sName[32];
// 		char			m_sParent[32];
// 		float			m_fRotation[3];
// 		float			m_fPosition[3];
		unsigned short	m_usNumRotFrames;
		unsigned short	m_usNumTransFrames;
		CKeyFrame*		m_pRotKeyFrames;
		CKeyFrame*		m_pTransKeyFrames;
		//////////////////////////////////////////////////////////////////////////
		short			m_nParent;
		CMatrix4		m_matLocal;
		CMatrix4		m_matAbs;
		CMatrix4		m_matFinal;
		unsigned short	m_usCurTransFrame;
		unsigned short	m_usCurRotFrame;
	};
}


#endif // !defined(JOINT_H_BC0F3116_FED0_45A7_B9D5_2179FF91C660)
