#ifndef ZBUFFER_H_ADB5D6F7_BBF9_4c4b_98B1_A7312F4A1527
#define ZBUFFER_H_ADB5D6F7_BBF9_4c4b_98B1_A7312F4A1527

#include <cstddef>

namespace Nomad3D
{
#define NM3D_ZBUFFER_MAX_ZVALUE	0x7fffffff
	extern inline void MemSet32(void *dest, unsigned int data, int count);

	class CZBuffer
	{
	public:
		CZBuffer()
		{
			m_pZBuffer = NULL;
		}
		~CZBuffer()
		{
			if(m_pZBuffer)
				delete[] m_pZBuffer;
		}
		void InitZBuffer(int width, int height)
		{
		#if (NM3D_RENDER_ZBUFFER_ON)
			if(m_pZBuffer)
				delete[] m_pZBuffer;

			m_pZBuffer = new int[width*height];

			m_nWidth = width;
			m_nHeight = height;

			ClearZBuffer();
		#endif
		}
		inline int* GetBuffer()
		{
			return m_pZBuffer;
		}
		inline int GetWidth()
		{
			return m_nWidth;
		}
		inline int GetHeight()
		{
			return m_nHeight;
		}
		inline void ClearZBuffer()
		{
			MemSet32(m_pZBuffer, NM3D_ZBUFFER_MAX_ZVALUE, m_nWidth*m_nHeight);
		}
	protected:
		int* m_pZBuffer;
		int m_nWidth;
		int m_nHeight;
	};
}

#endif //ZBUFFER_H_ADB5D6F7_BBF9_4c4b_98B1_A7312F4A1527
