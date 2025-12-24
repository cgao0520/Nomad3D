// testpicture.h: interface for the CTestPicture class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TESTPICTURE_H__
#define __TESTPICTURE_H__

#include "nomad3d.h"
#include <list>
#include "timer.h"

using namespace Nomad3D;

class CTestPicture : public CPicture  
{
public:
	CTestPicture();
	virtual ~CTestPicture();
	void Draw(int n=0);
	//////////////////////////////////////////////////////////////////////////
	void ResetTimer();
	inline void AddObject(CObject* pObject)
	{
		if(pObject)
		{
			m_ObjectList.push_back(pObject);
		}
	}
private:
	std::list<CObject*>	m_ObjectList;
	CTimer				m_Timer;
};

#endif // __TESTPICTURE_H__
