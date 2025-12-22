#ifndef CLOCK_MANAGER
#define CLOCK_MANAGER

//#include "platform.h"
#include "windows.h"

/*
#define FPS_30	33.3333
#define FPS_60	16.6667
#define FPS_100	10
#define FPS_200	5
*/
const DWORD FPS_1=(DWORD)1000;
const DWORD FPS_2=(DWORD)500;
const DWORD FPS_3=(DWORD)333.3333;
const DWORD FPS_5=(DWORD)200;
const DWORD FPS_10=(DWORD)100;
const DWORD FPS_30=(DWORD)33.3333;
const DWORD FPS_60=(DWORD)16.6667;
const DWORD FPS_100=(DWORD)10;
const DWORD FPS_200=(DWORD)5;

extern DWORD	start_clock_count;    // used for timing

DWORD Get_Clock(void);
DWORD Start_Clock(void);
DWORD Wait_Clock(DWORD count);

#endif