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
#include "WinScreenCapture_GDI+.h"
#include "WinScreenCapture_D3D9.h"
#include "WinScreenCapture_D3D11.h"
#include "WinScreenCapture_RDP.h"
#include "WinScreenCaptureHelper.h"
//-------------------------------------------------------------------------------------------------
#pragma warning (disable : 4996) // Remove ::sprintf() security warnings
//-------------------------------------------------------------------------------------------------
#define CHECK_CAP(_TCap_, _str_, _count_, _tag_, _img_) \
{ \
	LOG_INFO("+ Cheching " _tag_ "...\n"); \
	_TCap_  wsc; \
	if (wsc.captureScreenRect(0, 0, _img_.GetWidth(), _img_.GetHeight(), _img_)) \
	{ \
		_str_ += __pSeparator[_count_++ > 0]; \
		_str_ += "\"" _tag_ "\""; \
	} \
}
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

const char *WinScreenCaptureHelper::__pSeparator[2] = { " ", ", " };

const char *WinScreenCaptureHelper::__pHexValues = "0123456789ABCDEF";

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinScreenCaptureHelper::WinScreenCaptureHelper()
	: _strCapabilities(buildCapabilitiesString())
	, _strDisplayDevices(buildDisplayDevicesString())
{
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

IWinScreenCapture *WinScreenCaptureHelper::checkSettings(Settings &settings) const
{
	// Set display device
	const std::wstring strDevice = decodeURI(settings.strDevice);

	// Set capturer
	IWinScreenCapture *pScreenCapturer = nullptr;
	switch (settings.eCapturer)
	{
		case Capturer::None:
			break;
		case Capturer::Unknown: // Fallthrough
		case Capturer::GDI:
			if (_strCapabilities.find("\"GDI\"") != std::string::npos)
			{
				LOG_INFO("Using GDI capture...\n");
				pScreenCapturer = new WinScreenCapture_GDI(strDevice.c_str());
			}
			else LOG_ERROR("GDI capture not available!\n");
			break;
		case Capturer::GDIplus:
			if (_strCapabilities.find("\"GDI+\"") != std::string::npos)
			{
				LOG_INFO("Using GDI+ capturer...\n");
				pScreenCapturer = new WinScreenCapture_GDIplus(strDevice.c_str());
			}
			else LOG_ERROR("GDI+ capture not available!\n");
			break;
		case Capturer::D3D9:
			if (_strCapabilities.find("\"D3D9\"") != std::string::npos)
			{
				LOG_INFO("Using D3D9 capturer...\n");
				pScreenCapturer = new WinScreenCapture_D3D9(strDevice.c_str());
			}
			else LOG_ERROR("D3D9 capture not available!\n");
			break;
		case Capturer::D3D11:
			if (_strCapabilities.find("\"D3D11\"") != std::string::npos)
			{
				LOG_INFO("Using D3D11 capturer...\n");
				pScreenCapturer = new WinScreenCapture_D3D11(strDevice.c_str());
			}
			else LOG_ERROR("D3D11 capture not available!\n");
			break;
		case Capturer::RDP:
			if (_strCapabilities.find("\"RDP\"") != std::string::npos)
			{
				LOG_INFO("Using RDP capturer...\n");
				pScreenCapturer = new WinScreenCapture_RDP(strDevice.c_str());
			}
			else LOG_ERROR("RDP capture not available!\n");
			break;
	}

	// Adjust setting values
	uint32_t nSizeX, nSizeY;
	if (pScreenCapturer && pScreenCapturer->getCurrentScreenSize(nSizeX, nSizeY))
	{
		if (!settings.nCX || !settings.nCY ||
			(settings.nX0 + settings.nCX > nSizeX) ||
				(settings.nY0 + settings.nCY > nSizeY))
		{
			settings.nX0 = 0;       settings.nY0 = 0;
			settings.nCX = nSizeX;  settings.nCY = nSizeY;
		}
		if (settings.nWidth && !settings.nHeight)
		{
			settings.nHeight = (settings.nWidth * settings.nCY) / settings.nCX;
		}
		if (!settings.nWidth && settings.nHeight)
		{
			settings.nWidth = (settings.nHeight * settings.nCX) / settings.nCY;
		}
		if (!settings.nWidth && !settings.nHeight)
		{
			settings.nWidth = settings.nCX;
			settings.nHeight = settings.nCY;
		}
		if (!settings.nFPS)
		{
			settings.nFPS = 25;
		}

		// Take care about resizing aspect ratio
		uint32_t nDstWidth = settings.nCX;
		uint32_t nDstHeight = settings.nCY;
		if (nDstWidth > settings.nWidth)
		{
			nDstWidth = settings.nWidth;
			nDstHeight = (nDstWidth * settings.nCY) / settings.nCX;
		}
		if (nDstHeight > settings.nHeight)
		{
			nDstHeight = settings.nHeight;
			nDstWidth = (nDstHeight * settings.nCX) / settings.nCY;
		}

		// Set destination size
		settings.nWidth = nDstWidth;
		settings.nHeight = nDstHeight;
	}
	return pScreenCapturer;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

const std::string &WinScreenCaptureHelper::getCapabilitiesString() const
{
	return _strCapabilities;
}
//-------------------------------------------------------------------------------------------------

const std::string &WinScreenCaptureHelper::getDisplayDevicesString() const
{
	return _strDisplayDevices;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

std::string WinScreenCaptureHelper::buildCapabilitiesString()
{
	CImage imgTmp;
	imgTmp.Create(1, 1, 32);

	LOG_INFO("\nChecking capabilities...\n");
	uint32_t nCapabilities = 0;
	std::string strString = "[";
	CHECK_CAP(WinScreenCapture_GDI, strString, nCapabilities, "GDI", imgTmp);
	CHECK_CAP(WinScreenCapture_GDIplus, strString, nCapabilities, "GDI+", imgTmp);
	CHECK_CAP(WinScreenCapture_D3D9, strString, nCapabilities, "D3D9", imgTmp);
	CHECK_CAP(WinScreenCapture_D3D11, strString, nCapabilities, "D3D11", imgTmp);
	CHECK_CAP(WinScreenCapture_RDP, strString, nCapabilities, "RDP", imgTmp);
	strString += " ]";
	return strString;
}
//-------------------------------------------------------------------------------------------------

std::string WinScreenCaptureHelper::buildDisplayDevicesString()
{
	LOG_INFO("\nGetting devices and monitors...\n");
	DISPLAY_DEVICE displayDevice;
	ZeroMemory(&displayDevice, sizeof(DISPLAY_DEVICE));
	displayDevice.cb = sizeof(DISPLAY_DEVICE);

	std::string strString = "[";
	for (uint32_t nIndex = 0; EnumDisplayDevices(NULL, nIndex, &displayDevice, 0); ++nIndex)
	{
		strString += __pSeparator[nIndex > 0];
		strString += dumpDisplayDeviceInfo(displayDevice);
	}
	strString += " ]";
	return strString;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

std::string WinScreenCaptureHelper::dumpDisplayDeviceInfo(const DISPLAY_DEVICE &displayDevice)
{
	std::string strString;

	// Dump DisplayDevice info into the string
	strString += "{ \"name\" : \"";
	strString += encodeURI(displayDevice.DeviceName);
	strString += "\", \"description\" : \"";
	strString += encodeJSON(displayDevice.DeviceString);
	strString += "\", \"flags\" : [";

	// Parse state flags
	uint32_t nFlagCount = 0;
	if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"ACTIVE\"";
	}
	if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"MIRROR_DRIVER\"";
	}
	if (displayDevice.StateFlags & DISPLAY_DEVICE_MODESPRUNED)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"MODES_PRUNED\"";
	}
	if (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"PRIMARY_DEVICE\"";
	}
	if (displayDevice.StateFlags & DISPLAY_DEVICE_REMOVABLE)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"DEVICE_REMOVABLE\"";
	}
	if (displayDevice.StateFlags & DISPLAY_DEVICE_VGA_COMPATIBLE)
	{
		strString += __pSeparator[nFlagCount++ > 0];
		strString += "\"VGA_COMPATIBLE\"";
	}
	strString += " ], \"devices\" : [";

	// List all sub-devices (recursively)
	DISPLAY_DEVICE displayDevice2;
	ZeroMemory(&displayDevice2, sizeof(DISPLAY_DEVICE));
	displayDevice2.cb = sizeof(DISPLAY_DEVICE);
	for (uint32_t nIndex = 0; EnumDisplayDevices(displayDevice.DeviceName, nIndex, &displayDevice2, 0); ++nIndex)
	{
		strString += __pSeparator[nIndex > 0];
		strString += dumpDisplayDeviceInfo(displayDevice2);
	}
	strString += " ] }";

	return strString;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

std::string WinScreenCaptureHelper::encodeJSON(const wchar_t *strInput)
{
	std::string strString;
	for ( ; *strInput; ++strInput)
	{
		if ((*strInput >= 0x0020) && (*strInput <= 0x007F))
		{
			if ((*strInput == L'\\') || (*strInput == L'\"'))
			{
				strString += '\\';
			}
			strString += static_cast<char>(*strInput);
		}
		else
		{
			if (*strInput < 0x0020)
			{
				if (*strInput == L'\n')
					strString += "\\n";
				else if (*strInput == L'\r')
					strString += "\\r";
				else if (*strInput == L'\f')
					strString += "\\f";
				else if (*strInput == L'\t')
					strString += "\\t";
			}
			else
			{
				const uint8_t *pAux = reinterpret_cast<const uint8_t*>(strInput);
				strString += "\\u";
				strString += __pHexValues[(pAux[1] >> 4) & 0x0F];
				strString += __pHexValues[(pAux[1]) & 0x0F];
				strString += __pHexValues[(pAux[0] >> 4) & 0x0F];
				strString += __pHexValues[(pAux[0]) & 0x0F];
			}
		}
	}
	return strString;
}
//-------------------------------------------------------------------------------------------------

std::string WinScreenCaptureHelper::encodeURI(const wchar_t *strInput)
{
	char pMultiByte[4];
	std::string strString;
	for ( ; *strInput; ++strInput)
	{
		if (::iswalnum(*strInput) || (*strInput == L'_') || (*strInput == L'-') || (*strInput == L'.') || (*strInput == L'~'))
		{
			strString += static_cast<char>(*strInput);
		}
		else
		{
			const int nLength = WideCharToMultiByte(CP_UTF8, 0, strInput, 1, pMultiByte, 4, NULL, NULL);
			for (int n = 0; n < nLength; ++n)
			{
				strString += "%";
				strString += __pHexValues[(pMultiByte[n] >> 4) & 0x0F];
				strString += __pHexValues[(pMultiByte[n]) & 0x0F];
			}
		}
	}
	return strString;
}
//-------------------------------------------------------------------------------------------------

std::wstring WinScreenCaptureHelper::decodeURI(const char *strInput)
{
	uint8_t pMultiByte[4];
	wchar_t wChar;
	std::wstring strString;
	while (*strInput)
	{
		if (strInput[0] != '%')
		{
			strString += static_cast<wchar_t>(*strInput);
			++strInput;
		}
		else
		{
			const char *pA = ::strchr(__pHexValues, strInput[1]);
			const char *pB = ::strchr(__pHexValues, strInput[2]);
			bool bEnd = false;
			bool bError = !pA || !pB;
			uint32_t nCount = 0;
			if (!bError)
			{
				pMultiByte[0] = (static_cast<uint32_t>(pA - __pHexValues) << 4) + static_cast<uint32_t>(pB - __pHexValues);
				bEnd = (pMultiByte[0] < 0xC0);
				bError = !bEnd && (strInput[3] != '%');
				nCount = 1;
				if (!bEnd && !bError)
				{
					pA = ::strchr(__pHexValues, strInput[4]);
					pB = ::strchr(__pHexValues, strInput[5]);
					bError = !pA || !pB;
					if (!bError)
					{
						pMultiByte[1] = (static_cast<uint32_t>(pA - __pHexValues) << 4) + static_cast<uint32_t>(pB - __pHexValues);
						bEnd = (pMultiByte[0] < 0xE0);
						bError = (pMultiByte[1] < 0x80) || (pMultiByte[1] >= 0xC0) || (!bEnd && (strInput[6] != '%'));
						nCount = 2;
						if (!bEnd && !bError)
						{
							pA = ::strchr(__pHexValues, strInput[7]);
							pB = ::strchr(__pHexValues, strInput[8]);
							bError = !pA || !pB;
							if (!bError)
							{
								pMultiByte[2] = (static_cast<uint32_t>(pA - __pHexValues) << 4) + static_cast<uint32_t>(pB - __pHexValues);
								bEnd = (pMultiByte[0] < 0xF0);
								bError = (pMultiByte[2] < 0x80) || (pMultiByte[2] >= 0xC0) || (!bEnd && (strInput[9] != '%'));
								nCount = 3;
								if (!bEnd && !bError)
								{
									pA = ::strchr(__pHexValues, strInput[10]);
									pB = ::strchr(__pHexValues, strInput[11]);
									bError = !pA || !pB;
									if (!bError)
									{
										pMultiByte[3] = (static_cast<uint32_t>(pA - __pHexValues) << 4) + static_cast<uint32_t>(pB - __pHexValues);
										bEnd = true;
										bError = (pMultiByte[3] < 0x80) || (pMultiByte[3] >= 0xC0);
										nCount = 4;
									}
								}
							}
						}
					}
				}
			}
			if (bEnd && !bError && (MultiByteToWideChar(CP_UTF8, 0, (const char*)pMultiByte, nCount, &wChar, 1) > 0))
			{
				strString += wChar;
			}
			else
			{
				strString += nCount? L"???" : L"%";
				LOG_ERROR("decodeURI() invalid percent-encoding sequence! (%s)\n", strInput);
			}
			strInput += nCount? 3 * nCount : 1;
		}
	}
	return strString;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
