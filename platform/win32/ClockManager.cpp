
#include "ClockManager.h"

///////////////////////////////////////////////////////////

DWORD	start_clock_count = 0;     // used for timing

DWORD Get_Clock(void)
{
	// this function returns the current tick count

	// return time
	return(GetTickCount());

} // end Get_Clock

///////////////////////////////////////////////////////////

DWORD Start_Clock(void)
{
	// this function starts the clock, that is, saves the current
	// count, use in conjunction with Wait_Clock()

	return(start_clock_count = Get_Clock());

} // end Start_Clock

////////////////////////////////////////////////////////////

DWORD Wait_Clock(DWORD count)
{
	// this function is used to wait for a specific number of clicks
	// since the call to Start_Clock

	//while((Get_Clock() - start_clock_count) < count);

	int n=count - (Get_Clock() - start_clock_count);
	if(n>0)
		Sleep(n);

	return(Get_Clock());

} // end Wait_Clock

///////////////////////////////////////////////////////////