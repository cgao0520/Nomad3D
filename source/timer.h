// timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__DAA71C44_ECE5_415A_8850_13C0783927B8__INCLUDED_)
#define AFX_TIMER_H__DAA71C44_ECE5_415A_8850_13C0783927B8__INCLUDED_


#include "nomad3d.h"

#if defined(NM3D_PLATFORM_LINUX)
#include <time.h>
#endif

namespace Nomad3D
{
#if defined(NM3D_PLATFORM_WIN32)
#include "../platform/win32/timer.h"
#elif defined(NM3D_PLATFORM_LINUX)

    class CTimer
    {
    public:
        CTimer(){};
        virtual ~CTimer(){};

        void Init()
        {
            clock_gettime(CLOCK_MONOTONIC, &last_);
        }
        
        float GetSeconds()
        {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);

            float seconds =
                static_cast<float>(now.tv_sec - last_.tv_sec) +
                static_cast<float>(now.tv_nsec - last_.tv_nsec) * 1e-9f;

            last_ = now;
            return seconds;
        }

    protected:
        struct timespec last_;
    };
#else
    #error "CTimer class is not provided on this platform"
#endif
}

#endif // !defined(AFX_TIMER_H__DAA71C44_ECE5_415A_8850_13C0783927B8__INCLUDED_)
