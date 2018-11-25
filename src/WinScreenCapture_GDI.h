/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSCREENCAPTURE_GDI_H__
#define __WINSCREENCAPTURE_GDI_H__
//-------------------------------------------------------------------------------------------------
#include "IWinScreenCapture.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * Screen capture class for capturing using GDI
 */
class WinScreenCapture_GDI : public IWinScreenCapture
{
public:
	WinScreenCapture_GDI();
	~WinScreenCapture_GDI();

	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img);

private:
	HDC _hDCScreen;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSCREENCAPTURE_GDI_H__
//-------------------------------------------------------------------------------------------------
