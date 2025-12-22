// joint.cpp: implementation of the CJoint class.
//
//////////////////////////////////////////////////////////////////////

#include "joint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace Nomad3D
{
	CJoint::CJoint()
	{
		m_pRotKeyFrames = NULL;
		m_pTransKeyFrames = NULL;
	}

	CJoint::~CJoint()
	{
		if(m_pRotKeyFrames)
			delete[] m_pRotKeyFrames;
		if(m_pTransKeyFrames)
			delete[] m_pTransKeyFrames;
	}

	void CJoint::Assign(unsigned short nParentBoneIndex, const CMatrix4* pLocalMat, unsigned short usNumRotFrames, \
		unsigned short usNumTransFrames, const CKeyFrame* pRotKeyFrameData, const CKeyFrame* pTransKeyFrameData)
	{
		m_nParent = nParentBoneIndex;
// 		CMatrix4 matRot;
// 		CMatrix4 matTrans;
// 		CMatrix4::Rotate(fRotation[0], fRotation[1], fRotation[2], matRot);
// 		CMatrix4::Translate(fPosition[0], fPosition[1], fPosition[2], matTrans);
		m_matLocal = *pLocalMat;//matRot * matTrans;//Calculate the local matrix
		m_usNumRotFrames = usNumRotFrames;
		m_usNumTransFrames = usNumTransFrames;

		if(m_pRotKeyFrames)
			delete[] m_pRotKeyFrames;
		m_pRotKeyFrames = new CKeyFrame[m_usNumRotFrames];
		memcpy(m_pRotKeyFrames, pRotKeyFrameData, sizeof(CKeyFrame)*m_usNumRotFrames);

		if(m_pTransKeyFrames)
			delete[] m_pTransKeyFrames;
		m_pTransKeyFrames = new CKeyFrame[m_usNumTransFrames];
		memcpy(m_pTransKeyFrames, pTransKeyFrameData, sizeof(CKeyFrame)*m_usNumTransFrames);
	}
}
