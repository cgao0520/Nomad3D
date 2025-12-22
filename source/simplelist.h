// simplelist.h: interface for the CSimpleList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SIMPLELIST_H_145614D4_4F8E_49E4_9897_52893EEF7BD6
#define SIMPLELIST_H_145614D4_4F8E_49E4_9897_52893EEF7BD6

// use std::list for now
#include <list>

namespace Nomad3D
{
	template<class T>
		class CSimpleList
	{
public:
	typedef typename std::list<T>::iterator iterator;
	CSimpleList()
	{
	}
	//virtual ~CSimpleList();
	void PushBack(const T& ob)
	{
		m_List.push_back(ob);
	}
	void PushHead(const T& ob)
	{
		m_List.push_front(ob);
	}
	int GetSize()
	{
		return m_List.size();
	}
	iterator First()
	{
		return m_List.begin();
	}
	iterator Last()
	{
		return m_List.end();
	}
protected:
	std::list<T>	m_List;
	};
}


#endif //SIMPLELIST_H_145614D4_4F8E_49E4_9897_52893EEF7BD6
