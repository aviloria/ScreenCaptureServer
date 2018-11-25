/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture_RDP.h"
//-------------------------------------------------------------------------------------------------
#define MIRROR_DRIVER_NAME      _T("RDP Encoder Mirror Driver")
//-------------------------------------------------------------------------------------------------
#if defined(_DEBUG)
#  define LOG_DEBUG(...)  ::fprintf(stdout, __VA_ARGS__)
#else
#  define LOG_DEBUG(...)
#endif
#define LOG_INFO(...)  ::fprintf(stdout, __VA_ARGS__)
#define LOG_ERROR(...)  ::fprintf(stderr, __VA_ARGS__)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinScreenCapture_RDP::WinScreenCapture_RDP()
	: _hDCScreen(NULL)
{
	DISPLAY_DEVICE dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (UINT nIndex = 0; EnumDisplayDevices(NULL, nIndex, &dd, EDD_GET_DEVICE_INTERFACE_NAME) && !_hDCScreen; ++nIndex)
	{
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		if (EnumDisplaySettingsEx(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm, EDS_ROTATEDMODE) &&
			!_tccmp(dd.DeviceString, MIRROR_DRIVER_NAME))
		{
			_hDCScreen = ::CreateDC(dd.DeviceName, NULL, NULL, NULL);
		}
	}
}
//-------------------------------------------------------------------------------------------------

WinScreenCapture_RDP::~WinScreenCapture_RDP()
{
	// Clean up
	if (_hDCScreen)
	{
		::DeleteDC(_hDCScreen);
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_RDP::captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
{
	BOOL nRet = FALSE;
	if ((nSizeX > 0) && (nSizeY > 0) && !img.IsNull() && _hDCScreen)
	{
		// Stretch-blit from screen to image device context
		// Note: CAPTUREBLT flag is required to capture layered windows
		HDC hDCImage = img.GetDC();
		::SetStretchBltMode(hDCImage, HALFTONE);
		nRet = ::StretchBlt(hDCImage, 0, 0, img.GetWidth(), img.GetHeight(), _hDCScreen, nX0, nY0, nSizeX, nSizeY, SRCCOPY | CAPTUREBLT);
		img.ReleaseDC();
	}
	else LOG_ERROR("captureScreenRect() Invalid rect size (%ux%u), or target image was not initialized!\n", nSizeX, nSizeY);
	return nRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
