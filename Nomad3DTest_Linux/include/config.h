#ifndef CONFIG_H_ACCA488F_F977_4b19_8872_2701DE0BD0AB
#define CONFIG_H_ACCA488F_F977_4b19_8872_2701DE0BD0AB


#define NM3D_PLATFORM_LINUX						// current platform

#define NM3D_WINDOW_WIDTH						1024 // window width
#define NM3D_WINDOW_HEIGHT						768 // window height

#define NM3D_DEBUG_ON							1 // (0/1) if 1: print debug info to log file
#define NM3D_LIGHT_SPECULAR_ON					1 // (0/1) if 1: light with specular part(turn it off can boost run efficiency)
#define NM3D_LIGHT_SEPARATE_SPECULAR			1 // (0/1) if 1: light the specular part in separate and independent color in CPolygon class
#define NM3D_LIGHT_FLAT_SHADING_AVERAGE_POINT	0 // (0/1) if 1: calculate the average vertex of polygon when light in polygon mode 
#define NM3D_RENDER_ZBUFFER_ON					1 // (0/1) if 1: use Z buffer

#define NM3D_MAX_PICTURE_COUNT					4 // max number of pictures within one picture
//#define NM3D_MAX_PLAYER_COUNT					4 // max number of player within one application
#define NM3D_MAX_LIGHT_COUNT					4 // max number of lights within one light model

// Define the hardware RGB format where the Red (R) component is in the low-order bits.
// For Windows, R is in the high-order bits; for NDS, R is in the low-order bits
#define NM3D_PIXELFORMAT_R_LOW					0
//#define NM3D_PIXELFORMAT_888
//#define NM3D_PIXELFORMAT_555
#define NM3D_PIXELFORMAT_565

#ifdef NM3D_PIXELFORMAT_555
	#undef NM3D_PIXELFORMAT_565
	#undef NM3D_PIXELFORMAT_888
#elif defined(NM3D_PIXELFORMAT_565)
	#undef NM3D_PIXELFORMAT_555
	#undef NM3D_PIXELFORMAT_888
#elif defined(NM3D_PIXELFORMAT_888)
	#undef NM3D_PIXELFORMAT_555
	#undef NM3D_PIXELFORMAT_565
#endif


#endif //CONFIG_H_ACCA488F_F977_4b19_8872_2701DE0BD0AB
