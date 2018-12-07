/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture_D3D9.h"
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

WinScreenCapture_D3D9::WinScreenCapture_D3D9(const TCHAR *strDisplayDevice)
	: _nAdapterId(0xFFFFFFFF)
	, _pD3D(nullptr)
	, _pDevice(nullptr)
	, _pSurface(nullptr)
{
	_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (_pD3D)
	{
		// Get D3D9 Adapter Identifier
		if (strDisplayDevice)
		{
			// Convert strDisplayDevice from wchar to char (if needed)
#if defined(_UNICODE)
			char strTargetDisplayDevice[32] = { '\0' };
			INT nTargetDisplayDeviceLength = WideCharToMultiByte(CP_ACP, 0, strDisplayDevice, (int)::_tcslen(strDisplayDevice), strTargetDisplayDevice, 32, NULL, NULL);
			if (nTargetDisplayDeviceLength < 0) nTargetDisplayDeviceLength = 0;
			strTargetDisplayDevice[nTargetDisplayDeviceLength] = '\0';
#else
			const char *strTargetDisplayDevice = strDisplayDevice;
#endif

			// Look for the requested display device
			const UINT nMaxAdapters = _pD3D->GetAdapterCount();
			D3DADAPTER_IDENTIFIER9 adapterIdendifier;
			for (UINT nAdapter = 0; nAdapter < nMaxAdapters; ++nAdapter)
			{
				if (_pD3D->GetAdapterIdentifier(nAdapter, 0, &adapterIdendifier) == D3D_OK)
				{
					if (!::strcmp(strTargetDisplayDevice, adapterIdendifier.DeviceName))
					{
						_nAdapterId = nAdapter;
						break;
					}
				}
				else break;
			}
		}
		else _nAdapterId = D3DADAPTER_DEFAULT;

		// Initialize capturer
		D3DDISPLAYMODE mode;
		HRESULT hr = _pD3D->GetAdapterDisplayMode(_nAdapterId, &mode);
		if (SUCCEEDED(hr))
		{
			D3DPRESENT_PARAMETERS parameters;
			ZeroMemory(&parameters, sizeof(D3DPRESENT_PARAMETERS));
			parameters.BackBufferWidth  = mode.Width;
			parameters.BackBufferHeight = mode.Height;
			parameters.BackBufferFormat = mode.Format;
			parameters.SwapEffect       = D3DSWAPEFFECT_DISCARD;
			parameters.Windowed         = TRUE;

			hr = _pD3D->CreateDevice(_nAdapterId, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &_pDevice);
			if (SUCCEEDED(hr))
			{
				hr = _pDevice->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &_pSurface, nullptr);
				if (!SUCCEEDED(hr))
				{
					LOG_ERROR("CreateOffscreenPlainSurface() returned error code 0x%08x!\n", (UINT)hr);
				}
			}
			else LOG_ERROR("CreateDevice() returned error code 0x%08x!\n", (UINT)hr);
		}
		else LOG_ERROR("GetFrontBufferData() returned error code 0x%08x!\n", (UINT)hr);
	}
	else LOG_ERROR("Direct3DCreate9() failed!\n");
}
//-------------------------------------------------------------------------------------------------

WinScreenCapture_D3D9::~WinScreenCapture_D3D9()
{
	// Clean up
	if (_pSurface)
	{
		_pSurface->Release();
	}
	if (_pDevice)
	{
		_pDevice->Release();
	}
	if (_pD3D)
	{
		_pD3D->Release();
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_D3D9::getCurrentScreenSize(UINT &nSizeX, UINT &nSizeY) const
{
	D3DDISPLAYMODE mode;
	if (_pD3D && SUCCEEDED(_pD3D->GetAdapterDisplayMode(_nAdapterId, &mode)))
	{
		nSizeX = mode.Width;
		nSizeY = mode.Height;
		return TRUE;
	}
	return FALSE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_D3D9::captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
{
	BOOL nRet = FALSE;
	HDC hDCScreen = NULL;
	if ((nSizeX > 0) && (nSizeY > 0) && !img.IsNull() && _pSurface)
	{
		HRESULT hr = _pDevice->GetFrontBufferData(0, _pSurface);
		if (SUCCEEDED(hr))
		{
			hr = _pSurface->GetDC(&hDCScreen);
			if (SUCCEEDED(hr))
			{
				// Stretch-blit from screen to image device context
				// Note: CAPTUREBLT flag is required to capture layered windows
				HDC hDCImage = img.GetDC();
				::SetStretchBltMode(hDCImage, HALFTONE);
				nRet = ::StretchBlt(hDCImage, 0, 0, img.GetWidth(), img.GetHeight(), hDCScreen, nX0, nY0, nSizeX, nSizeY, SRCCOPY | CAPTUREBLT);
				img.ReleaseDC();

				// Clean up
				_pSurface->ReleaseDC(hDCScreen);
			}
			else LOG_ERROR("GetDC() returned error code 0x%08x!\n", (UINT)hr);
		}
		else LOG_ERROR("GetFrontBufferData() returned error code 0x%08x!\n", (UINT)hr);
	}
	else LOG_ERROR("captureScreenRect() Invalid rect size (%ux%u), target image was not initialized, or D3D9 surface was not set!\n", nSizeX, nSizeY);
	return nRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
