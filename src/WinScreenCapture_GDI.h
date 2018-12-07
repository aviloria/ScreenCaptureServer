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
	WinScreenCapture_GDI(const TCHAR *strDisplayDevice=NULL);
	~WinScreenCapture_GDI();

	BOOL getCurrentScreenSize(UINT &nSizeX, UINT &nSizeY) const;

	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img);

private:
	HDC _hDCScreen;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSCREENCAPTURE_GDI_H__
//-------------------------------------------------------------------------------------------------
