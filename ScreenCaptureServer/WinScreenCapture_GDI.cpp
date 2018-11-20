/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture_GDI.h"
//-------------------------------------------------------------------------------------------------
#if defined(_DEBUG)
#  define LOG(...)  ::fprintf(stdout, __VA_ARGS__)
#else
#  define LOG(...)
#endif
#define LOG_ERROR(...)  ::fprintf(stderr, __VA_ARGS__)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinScreenCapture_GDI::WinScreenCapture_GDI()
{
}
//-------------------------------------------------------------------------------------------------

WinScreenCapture_GDI::~WinScreenCapture_GDI()
{
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_GDI::captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
{
	BOOL nRet = FALSE;
	if ((nSizeX > 0) && (nSizeY > 0) && !img.IsNull())
	{
		// Create a screen device context
		//HDC hDCScreen = GetDC(NULL); // HDC only of primary monitor
		HDC hDCScreen = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL); // Create a DC covering all the monitors

		// Stretch-blit from screen to image device context
		// Note: CAPTUREBLT flag is required to capture layered windows
		HDC hDCImage = img.GetDC();
		::SetStretchBltMode(hDCImage, HALFTONE);
		nRet = ::StretchBlt(hDCImage, 0, 0, img.GetWidth(), img.GetHeight(), hDCScreen, nX0, nY0, nSizeX, nSizeY, SRCCOPY | CAPTUREBLT);
		img.ReleaseDC();

		// Clean up
		::DeleteDC(hDCScreen);
	}
	else LOG_ERROR("captureScreenRect() Invalid rect size (%ux%u), or target image was not initialized!\n", nSizeX, nSizeY);
	return nRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
