/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSCREENCAPTURE_RDP_H__
#define __WINSCREENCAPTURE_RDP_H__
//-------------------------------------------------------------------------------------------------
#include "IWinScreenCapture.h"
#include <Windows.h>
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * Screen capture class for capturing using RDP Mirror Driver (Windows 7)
 */
class WinScreenCapture_RDP : public IWinScreenCapture
{
public:
	WinScreenCapture_RDP();
	~WinScreenCapture_RDP();

	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img);

private:
	HDC _hDCScreen;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSCREENCAPTURE_RDP_H__
//-------------------------------------------------------------------------------------------------
