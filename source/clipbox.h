#ifndef CLIP_BOX_919A20C7_8EEB_41d2_AE81_AC9C1938F8C6
#define CLIP_BOX_919A20C7_8EEB_41d2_AE81_AC9C1938F8C6

namespace Nomad3D
{

#define NM3D_CLIP_BOX_VAL \
	int min_clip_x;\
	int max_clip_x;\
	int	min_clip_y;\
	int max_clip_y;
	
#define NM3D_CLIP_MIN	0
#define NM3D_CLIP_MAX	9999
	
	class CClipBox
	{
		friend class CRender;
		friend class CRenderList;

	public:

		CClipBox(){};

		CClipBox(const CClipBox& cb)
		{
			Assign(cb.min_clip_x, cb.max_clip_x, cb.min_clip_y, cb.max_clip_y);
		}

		CClipBox(int min_x, int max_x, int min_y, int max_y)
		{
			Assign(min_x, max_x, min_y, max_y);
		}

		void Assign(int min_x, int max_x, int min_y, int max_y)
		{
			min_clip_x = min_x;
			max_clip_x = max_x;
			min_clip_y = min_y;
			max_clip_y = max_y;
		}

		CClipBox& operator = (const CClipBox& cb)
		{
			Assign(cb.min_clip_x, cb.max_clip_x, cb.min_clip_y, cb.max_clip_y);
			return *this;
		}

		bool operator == (const CClipBox& cb) const
		{
			if(
				cb.min_clip_x == min_clip_x &&
				cb.max_clip_x == max_clip_x &&
				cb.min_clip_y == min_clip_y &&
				cb.max_clip_y == max_clip_y
				)
				return true;
			else
				return false;
		}

	private:

		NM3D_CLIP_BOX_VAL
	};
}

#endif
