/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture_GDI+.h"
#include <gdiplus.h>
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

const char *getStatusStr(const Gdiplus::Status &st)
{
	switch (st)
	{
		case Gdiplus::Status::Ok: return "Ok";
		case Gdiplus::Status::GenericError: return "GenericError";
		case Gdiplus::Status::InvalidParameter: return "InvalidParameter";
		case Gdiplus::Status::OutOfMemory: return "OutOfMemory";
		case Gdiplus::Status::ObjectBusy: return "ObjectBusy";
		case Gdiplus::Status::InsufficientBuffer: return "InsufficientBuffer";
		case Gdiplus::Status::NotImplemented: return "NotImplemented";
		case Gdiplus::Status::Win32Error: return "Win32Error";
		case Gdiplus::Status::WrongState: return "WrongState";
		case Gdiplus::Status::Aborted: return "Aborted";
		case Gdiplus::Status::FileNotFound: return "FileNotFound";
		case Gdiplus::Status::ValueOverflow: return "ValueOverflow";
		case Gdiplus::Status::AccessDenied: return "AccessDenied";
		case Gdiplus::Status::UnknownImageFormat: return "UnknownImageFormat";
		case Gdiplus::Status::FontFamilyNotFound: return "FontFamilyNotFound";
		case Gdiplus::Status::FontStyleNotFound: return "FontStyleNotFound";
		case Gdiplus::Status::NotTrueTypeFont: return "NotTrueTypeFont";
		case Gdiplus::Status::UnsupportedGdiplusVersion: return "UnsupportedGdiplusVersion";
		case Gdiplus::Status::GdiplusNotInitialized: return "GdiplusNotInitialized";
		case Gdiplus::Status::PropertyNotFound: return "PropertyNotFound";
		case Gdiplus::Status::PropertyNotSupported: return "PropertyNotSupported";
#if (GDIPVER >= 0x0110)
		case Gdiplus::Status::ProfileNotFound: return "ProfileNotFound";
#endif
	}
	return "Unknown!";
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinScreenCapture_GDIplus::WinScreenCapture_GDIplus(const TCHAR *strDisplayDevice)
	: _gdiplusToken(NULL)
	, _hDCScreen(NULL)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status st = Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
	if (st == Gdiplus::Status::Ok)
	{
		if (!strDisplayDevice)
			strDisplayDevice = _T("DISPLAY");

		// Create a screen device context
		//_hDCScreen = GetDC(NULL); // HDC only of primary monitor
		_hDCScreen = ::CreateDC(strDisplayDevice, NULL, NULL, NULL); // Create a DC covering all the monitors
	}
	else LOG_ERROR("GdiplusStartup() returned '%s'", getStatusStr(st));
}
//-------------------------------------------------------------------------------------------------

WinScreenCapture_GDIplus::~WinScreenCapture_GDIplus()
{
	// Clean up
	if (_hDCScreen)
	{
		::DeleteDC(_hDCScreen);
	}
	Gdiplus::GdiplusShutdown(_gdiplusToken);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_GDIplus::getCurrentScreenSize(UINT &nSizeX, UINT &nSizeY) const
{
	nSizeX = ::GetSystemMetrics(SM_CXSCREEN);
	nSizeY = ::GetSystemMetrics(SM_CYSCREEN);
	return TRUE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_GDIplus::captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
{
	BOOL nRet = FALSE;
	if ((nSizeX > 0) && (nSizeY > 0) && !img.IsNull())
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
