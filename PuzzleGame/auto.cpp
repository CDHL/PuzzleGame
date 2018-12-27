#include "stdafx.h"

#include "auto.h"

#include <vector>

template<typename DATA_TYPE>
class Allocator
{
	static const int ALLOC_COUNT = 1024;
	std::vector<DATA_TYPE*> m_pool;
	std::vector<DATA_TYPE*> m_recycle;
	int m_curElement;

public:

	Allocator() :m_curElement(ALLOC_COUNT) {}

	~Allocator()
	{
		for (DATA_TYPE *p : m_pool) delete[] p;
	}

	DATA_TYPE* getNew()
	{
		if (!m_recycle.empty())
		{
			DATA_TYPE* res = m_recycle.back();
			m_recycle.pop_back();
			return res;
		}
		if (m_curElement == ALLOC_COUNT)
		{
			m_pool.push_back(new DATA_TYPE[ALLOC_COUNT]);
			m_curElement = 0;
		}
		return m_pool.back() + m_curElement++;
	}

	void recycle(DATA_TYPE *p)
	{
		m_recycle.push_back(p);
	}
};

