/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

namespace WinScreenCapture
{
	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
	{
		BOOL nRet = (nSizeX > 0) && (nSizeY > 0);
		if (nRet)
		{
			// Check for proper image size
			if (img.IsNull() /*|| (img.GetWidth() != nSizeX) || (img.GetHeight() != nSizeY)*/)
			{
				if (!img.IsNull()) img.Destroy();
				img.Create(nSizeX, nSizeY, 24);
			}

			// Create a screen device context
			HDC hDCScreen = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

			// Bit-blit from screen to image device context
			// Note: CAPTUREBLT flag is required to capture layered windows
			HDC imageHDC = img.GetDC();
			//nRet = ::BitBlt(imageHDC, nX0, nY0, nSizeX, nSizeY, hDCScreen, 0, 0, SRCCOPY | CAPTUREBLT);
			nRet = ::StretchBlt(imageHDC, 0, 0, img.GetWidth(), img.GetHeight(), hDCScreen, nX0, nY0, nSizeX, nSizeY, SRCCOPY | CAPTUREBLT);
			img.ReleaseDC();

			// Perform cleanup
			::DeleteDC(hDCScreen);
		}
		return nRet;
	}
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
