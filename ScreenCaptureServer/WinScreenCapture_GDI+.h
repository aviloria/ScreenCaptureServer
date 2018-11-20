/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSCREENCAPTURE_GDIPLUS_H__
#define __WINSCREENCAPTURE_GDIPLUS_H__
//-------------------------------------------------------------------------------------------------
#include "IWinScreenCapture.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
* Namespace to hold different screen capture methods
*/
class WinScreenCapture_GDIplus : public IWinScreenCapture
{
public:
	WinScreenCapture_GDIplus();
	~WinScreenCapture_GDIplus();

	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img);

private:
	ULONG_PTR _gdiplusToken;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSCREENCAPTURE_GDIPLUS_H__
//-------------------------------------------------------------------------------------------------
