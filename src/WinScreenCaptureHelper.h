/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSCREENCAPTUREHELPER_H__
#define __WINSCREENCAPTUREHELPER_H__
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
#include <string>
#include "IWinScreenCapture.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
* Class to help working with different configurations
*/
class WinScreenCaptureHelper
{
public:
	enum class Capturer { Unknown, None, GDI, GDIplus, D3D9, D3D11, RDP };

	struct Settings
	{
		uint32_t nWidth = 0;                     // Destination image width (in pixels)
		uint32_t nHeight = 0;                    // Destination image height (in pixels)
		uint32_t nX0 = 0;                        // Snapshot offset in X-axis
		uint32_t nY0 = 0;                        // Snapshot offset in Y-axis
		uint32_t nCX = 0;                        // Snapshot width
		uint32_t nCY = 0;                        // Snapshot height
		uint32_t nFPS = 0;                       // Frame rate (video only)
		Capturer eCapturer = Capturer::Unknown;  // Capturer type to be used
		char strDevice[256] = { '\0' };          // Target display device
	};

public:
	WinScreenCaptureHelper();

	IWinScreenCapture *checkSettings(Settings &settings) const;

	const std::string &getCapabilitiesString() const;
	const std::string &getDisplayDevicesString() const;

private:
	static std::string buildCapabilitiesString();
	static std::string buildDisplayDevicesString();

	static std::string dumpDisplayDeviceInfo(const DISPLAY_DEVICE &displayDevice);
	static std::string encodeJSON(const wchar_t *strInput);
	static std::string encodeURI(const wchar_t *strInput);
	static std::wstring decodeURI(const char *strInput);

private:
	static const char *__pSeparator[2];
	static const char *__pHexValues;
	std::string _strCapabilities;     // Capabilities string
	std::string _strDisplayDevices;   // Display-devices string
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __IWINSCREENCAPTURE_H__
//-------------------------------------------------------------------------------------------------
