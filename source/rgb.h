// rgb.h: interface for the CRGB class.
//
//////////////////////////////////////////////////////////////////////

#ifndef RGB_H_45779666_74E3_48DA_92CB_DD36F3467C37
#define RGB_H_45779666_74E3_48DA_92CB_DD36F3467C37

#include "config.h"
#include "platform.h"
#include "constant.h"

namespace Nomad3D
{
	typedef unsigned short	RGB16;
	typedef unsigned int	RGB32;
	typedef unsigned char	u8;
	
	class CRGB  
	{
	public:
		inline CRGB()
		{
			rgb_ = 0;
		}

		inline CRGB(u8 r, u8 g, u8 b)
		{
		#if defined(NM3D_PIXELFORMAT_555)

			#if NM3D_PIXELFORMAT_R_LOW
				rgb_ = (r>>3) | ((g>>3)<<5) | ((b>>3)<<10);
			#else
				rgb_ = (b>>3) | ((g>>3)<<5) | ((r>>3)<<10);
			#endif

		#elif defined(NM3D_PIXELFORMAT_565)

			#if NM3D_PIXELFORMAT_R_LOW
				rgb_ = (r>>3) | ((g>>2)<<5) | ((b>>3)<<11);
			#else
				rgb_ = (b>>3) | ((g>>2)<<5) | ((r>>3)<<11);
			#endif

		#elif defined(NM3D_PIXELFORMAT888)

			#if NM3D_PIXELFORMAT_R_LOW
				rgb_ = (r) | (g<<8) | (b<<16);
			#else
				rgb_ = (b) | (g<<8) | (r<<16);
			#endif

		#endif	
		}

		inline u8 r() const
		{
		#if defined(NM3D_PIXELFORMAT_555)

			#if NM3D_PIXELFORMAT_R_LOW
				return rgb_ & 0x001f;
			#else
				return (rgb_ & 0x7c00) >> 10;
			#endif

		#elif defined(NM3D_PIXELFORMAT_565)

			#if NM3D_PIXELFORMAT_R_LOW
				return rgb_ & 0x001f;
			#else
				return (rgb_ & 0xf800) >> 11;
			#endif

		#elif defined(NM3D_PIXELFORMAT888)

			#if NM3D_PIXELFORMAT_R_LOW
				return rgb_ & 0x000000ff;
			#else
				return (rgb_ & 0x00ff0000) >> 16;
			#endif

		#endif
		}

		inline u8 g() const
		{
			//u8 part_r;
		#if defined(NM3D_PIXELFORMAT_555)

			return (rgb_ & 0x03e0) >> 5;

		#elif defined(NM3D_PIXELFORMAT_565)

			return (rgb_ & 0x07e0) >> 5;

		#elif defined(NM3D_PIXELFORMAT888)

			return (rgb_ & 0x0000ff00) >> 8;

		#endif
		}

		inline u8 b() const
		{
			//u8 part_r;
		#if defined(NM3D_PIXELFORMAT_555)

			#if NM3D_PIXELFORMAT_R_LOW
				return (rgb_ & 0x7c00) >> 10;
			#else
				return rgb_ & 0x001f;
			#endif

		#elif defined(NM3D_PIXELFORMAT_565)

			#if NM3D_PIXELFORMAT_R_LOW
				return (rgb_ & 0xf800) >> 11;
			#else
				return rgb_ & 0x001f;
			#endif

		#elif defined(NM3D_PIXELFORMAT888)

			#if NM3D_PIXELFORMAT_R_LOW
				return (rgb_ & 0x00ff0000) >> 16;
			#else
				return rgb_ & 0x000000ff;
			#endif

		#endif
		}

		inline u8 rc() const
		{
		#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
			return r()<<3;
		#elif defined(NM3D_PIXELFORMAT888)
			return r();
		#endif
		}

		inline u8 gc() const
		{
		#if defined(NM3D_PIXELFORMAT_555)
			return g()<<3;
		#elif defined(NM3D_PIXELFORMAT_565)
			return g()<<2;
		#elif defined(NM3D_PIXELFORMAT888)
			return g();
		#endif
		}

		inline u8 bc() const
		{
		#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
			return b()<<3;
		#elif defined(NM3D_PIXELFORMAT888)
			return b();
		#endif
		}

	public:

		inline CRGB operator * (float alpha)
		{
			if(alpha <0 || alpha>1)
				return *this;
			u8 r = rc()*alpha;
			u8 g = gc()*alpha;
			u8 b = bc()*alpha;
			CRGB color(r,g,b);
			return color;
		}
		
		inline CRGB operator + (CRGB color) const
		{
			u8 r = MIN(rc() + color.rc(),255);
			u8 g = MIN(gc() + color.gc(),255);
			u8 b = MIN(bc() + color.bc(),255);
			CRGB clr(r,g,b);
			return clr;
		}


#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)
		inline CRGB operator + (RGB16 color) const
		{
			CRGB clr(color);
			return operator+(clr);
		}

		inline CRGB(RGB16 rgb)
		{
			rgb_ = rgb;
		}

// 		inline RGB16* operator& ()
// 		{
// 			return &rgb_;
// 		}

		inline CRGB& operator = (RGB16 rgb)
		{
			rgb_ = rgb;
			return *this;
		}

		inline bool operator == (RGB16 rgb) const
		{
			return rgb_ == rgb;
		}

		inline RGB16 GetRGB()
		{
			return rgb_;
		}

		inline void GetRGBValue(u8& rr, u8& gg, u8& bb) const
		{
			rr = r();
			gg = g();
			bb = b();
		}

		inline void GetRGBValue(unsigned int& rr, unsigned int& gg, unsigned int& bb) const
		{
			rr = r();
			gg = g();
			bb = b();
		}

		inline void GetRGBValue(int& rr, int& gg, int& bb) const
		{
			rr = r();
			gg = g();
			bb = b();
		}

		inline void GetRGBValuec(u8& rr, u8& gg, u8& bb) const
		{
			rr = rc();
			gg = gc();
			bb = bc();
		}

		inline void GetRGBValuec(unsigned int& rr, unsigned int& gg, unsigned int& bb) const
		{
			rr = rc();
			gg = gc();
			bb = bc();
		}

		inline void GetRGBValuec(int& rr, int& gg, int& bb) const
		{
			rr = rc();
			gg = gc();
			bb = bc();
		}

		static inline RGB16 GenRGBFixp(int rr, int gg, int bb, int right_shiftbit)
		{
			#if defined (NM3D_PIXELFORMAT_555)
				#if (NM3D_PIXELFORMAT_R_LOW)
					return ((bb >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 10) + 
						((gg >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 5) + 
						(rr >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3));
				#else
					return ((rr >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 10) + 
						((gg >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 5) + 
						(bb >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3));
				#endif
			#elif defined(NM3D_PIXELFORMAT_565)
				#if (NM3D_PIXELFORMAT_R_LOW)
					return ((bb >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 11) + 
						((gg >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+2)) << 5) + 
						(rr >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3));
				#else
					return ((rr >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3)) << 11) + 
						((gg >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+2)) << 5) + 
						(bb >> (/*NM3D_FIXP16_SHIFT+8*/right_shiftbit+3));
				#endif
			#endif
		}

#elif defined(NM3D_PIXELFORMAT_888)
		inline CRGB operator + (RGB32 color)
		{
			CRGB clr(color);
			return operator+(clr);
		}

		inline RGB32 GetRGB()
		{
			return rgb_;
		}

		inline CRGB(RGB32 rgb)
		{
			rgb_ = rgb;
		}

// 		inline RGB32* operator& ()
// 		{
// 			return &rgb_;
// 		}

		inline CRGB& operator = (RGB32 rgb)
		{
			rgb_ = rgb;
			return *this;
		}

		inline bool operator == (RGB32 rgb)
		{
			return rgb_ == rgb;
		}

		static inline RGB32 GenRGBFixp(int r, int g, int b, int right_shiftbit)
		{
		#if (NM3D_PIXELFORMAT_R_LOW)
			return ((b>>right_shiftbit)<<16) + ((g>>right_shiftbit)<<8) + (r>>right_shiftbit);
		#else
			return ((r>>right_shiftbit)<<16) + ((g>>right_shiftbit)<<8) + (b>>right_shiftbit);
		#endif
		}

#endif

		inline unsigned char operator[] (int i) const
		{
			unsigned char part = 0;

			#if defined(NM3D_PIXELFORMAT_555)
				if(i == 0)
					part = rgb_ & 0x001f;
				else if (i == 1)
					part = (rgb_ & 0x03e0) >> 5;
				else if (i == 2)
					part = (rgb_ & 0x7c00) >> 10;
			#elif defined(NM3D_PIXELFORMAT_565)
				if(i == 0)
					part = rgb_ & 0x001f;
				else if (i == 1)
					part = (rgb_ & 0x07e0) >> 5;
				else if (i == 2)
					part = (rgb_ & 0xf800) >> 11;
			#elif defined(NM3D_PIXELFORMAT888)
				if(i == 0)
					part = rgb_ & 0x000000ff;
				else if (i == 1)
					part = (rgb_ & 0x0000ff00) >> 8;
				else if (i == 2)
					part = (rgb_ & 0x00ff0000) >> 16;
				else if (i == 3)
					part = (rgb_ & 0xff000000) >> 24;
			#endif
			return part;
		}

	protected:
		
#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)

		RGB16 rgb_;

#elif defined(NM3D_PIXELFORMAT_888)

		RGB32 rgb_;//a(alpha) is always at higher byte (MSB)

#endif

	};

//////////////////////////////////////////////////////////////////////////
//typedef CRGB16 and CRGBA32
#if defined(NM3D_PIXELFORMAT_555) || defined(NM3D_PIXELFORMAT_565)

	typedef	CRGB	CRGB16;

#elif defined(NM3D_PIXELFORMAT_888)

	typedef CRGB	CRGBA32;

#endif

	class CRGBA : public CRGB
	{
	public:
		CRGBA() : CRGB()
		{
			a_ = 1;
		}

		CRGBA(u8 r, u8 g, u8 b, u8 a=1) : CRGB(r, g, b)
		{
#if defined(NM3D_PIXELFORMAT_888)
			unsigned int temp = rgb_ & 0x00ffffff;
			rgb_ = temp || a<<24;
#else
			a_ = a;
#endif
		}

		inline u8& a()
		{
#if defined(NM3D_PIXELFORMAT_888)
			return rgb_>>24 & 0xff;
#else
			return a_;
#endif
		}

	protected:

		u8 a_;

	};
}


#endif // RGB_H_45779666_74E3_48DA_92CB_DD36F3467C37
