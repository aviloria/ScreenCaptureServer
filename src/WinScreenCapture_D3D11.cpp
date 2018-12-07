/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include "WinScreenCapture_D3D11.h"
#include <thread> // Needed for sleep_for
//-------------------------------------------------------------------------------------------------
// DXGI error codes: https://docs.microsoft.com/es-es/windows/desktop/direct3ddxgi/dxgi-error
//-------------------------------------------------------------------------------------------------
#if defined(_DEBUG)
#  define LOG_DEBUG(...)  ::fprintf(stdout, __VA_ARGS__)
#else
#  define LOG_DEBUG(...)
#endif
#define LOG_INFO(...)  ::fprintf(stdout, __VA_ARGS__)
#define LOG_ERROR(...)  ::fprintf(stderr, __VA_ARGS__)
//-------------------------------------------------------------------------------------------------
#define DUPLICATE_OUTPUT_RETRIES   10
#define DUPLICATE_OUTPUT_WAITTIME  50 // In milliseconds
#define AQUIRE_TIMEOUT             10 // In milliseconds
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinScreenCapture_D3D11::WinScreenCapture_D3D11(const TCHAR *strDisplayDevice)
	: _pD3D11Device(nullptr)
	, _pD3D11Context(nullptr)
	, _pDxgiOutput(nullptr)
	, _pDxgiOutputDuplication(nullptr)
	, _pTexture(nullptr)
{
	IDXGIAdapter1 *pDxgiAdapterSelected = nullptr;
	IDXGIOutput   *pDxgiOutputSelected = nullptr;
	DXGI_OUTPUT_DESC outputDesc;

	// Look for the output instance currently attached to desktop.
	IDXGIFactory1* pDxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (LPVOID*)&pDxgiFactory);
	if (SUCCEEDED(hr))
	{
		IDXGIAdapter1 *pDxgiAdapter = nullptr;
		for (UINT nAdapterId = 0; !pDxgiAdapterSelected; ++nAdapterId)
		{
			hr = pDxgiFactory->EnumAdapters1(nAdapterId, &pDxgiAdapter);
			if(SUCCEEDED(hr))
			{
				IDXGIOutput *pDxgiOutput = nullptr;
				for (UINT nOutputId = 0; !pDxgiOutputSelected; ++nOutputId)
				{
					hr = pDxgiAdapter->EnumOutputs(nOutputId, &pDxgiOutput);
					if(SUCCEEDED(hr))
					{
						hr = pDxgiOutput->GetDesc(&outputDesc);
						if (SUCCEEDED(hr))
						{
							LOG_INFO("  %2u> DeviceName: %S\n", nOutputId, outputDesc.DeviceName);
							if ((!strDisplayDevice && outputDesc.AttachedToDesktop) ||
								!::_tcscmp(strDisplayDevice, outputDesc.DeviceName))
							{
								pDxgiAdapterSelected = pDxgiAdapter;
								pDxgiOutputSelected  = pDxgiOutput;

								// Break here to avoid to release these interfaces
								break;
							}
						}
						else LOG_ERROR("GetDesc() Failed to get DXGI_OUTPUT_DESC for IDXGIOutput interface %u at IDXGIAdapter1 interdace %u... ignoring it! hr=0x%08x\n", nOutputId, nAdapterId,(UINT)hr);
						pDxgiOutput->Release();
					}
					else if(hr != DXGI_ERROR_NOT_FOUND)
						LOG_ERROR("EnumOutputs() Failed to get IDXGIOutput interface %u at IDXGIAdapter1 interdace %u... ignoring it! hr=0x%08x\n", nOutputId, nAdapterId, (UINT)hr);
					else break;
				}
				if (!pDxgiOutputSelected) pDxgiAdapter->Release();
			}
			else if(hr != DXGI_ERROR_NOT_FOUND)
				LOG_ERROR("EnumAdapters1() Failed to get IDXGIAdapter1 interface %u... ignoring it! hr=0x%08x\n", nAdapterId,(UINT)hr);
			else break;
		}
		pDxgiFactory->Release();
	}
	else LOG_ERROR("CreateDXGIFactory1() failed to retrieve the IDXGIFactory! hr=0x%08x\n", (UINT)hr);


	if (pDxgiAdapterSelected && pDxgiOutputSelected)
	{
		// NOTE: Apparently D3D11CreateDevice() fails when passing a pointer to an adapter (1st param) with D3D_DRIVER_TYPE_HARDWARE (2nd param).
		// To be able to pass a valid pointer for the adapter, you need to use D3D_DRIVER_TYPE_UNKNOWN.
		D3D_FEATURE_LEVEL featureLevel;
		hr = D3D11CreateDevice(pDxgiAdapterSelected, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED, nullptr, 0, D3D11_SDK_VERSION, &_pD3D11Device, &featureLevel, &_pD3D11Context);
		if (SUCCEEDED(hr))
		{
			if (featureLevel >= D3D_FEATURE_LEVEL_11_0)
			{
				hr = pDxgiOutputSelected->QueryInterface(__uuidof(IDXGIOutput1), (LPVOID*)&_pDxgiOutput);
				if (SUCCEEDED(hr))
				{
					// When we are initializing the DXGI, retrying several times to avoid any temporary issue, such as display mode changing,
					// to block us from using DXGI based capturer.
					for (unsigned int nRetry = 0; (nRetry < DUPLICATE_OUTPUT_RETRIES) && !_pDxgiOutputDuplication; ++nRetry)
					{
						if (!duplicateOutput())
							std::this_thread::sleep_for(std::chrono::milliseconds(DUPLICATE_OUTPUT_WAITTIME));
					}
					_pDxgiOutput->Release();  _pDxgiOutput = nullptr;
					if (_pDxgiOutputDuplication)
					{
						// Create the staging texture that we need to download the pixels from gpu
						D3D11_TEXTURE2D_DESC textureDesc;
						textureDesc.Width = outputDesc.DesktopCoordinates.right;
						textureDesc.Height = outputDesc.DesktopCoordinates.bottom;
						textureDesc.MipLevels = 1;
						textureDesc.ArraySize = 1; // When using a texture array
						textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Default format for desktop duplications: https://msdn.microsoft.com/en-us/library/windows/desktop/hh404611(v=vs.85).aspx
						textureDesc.SampleDesc.Count = 1; // MultiSampling, we can use 1 as we're just downloading an existing one.
						textureDesc.SampleDesc.Quality = 0;
						textureDesc.Usage = D3D11_USAGE_STAGING;
						textureDesc.BindFlags = 0;
						textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
						textureDesc.MiscFlags = 0;
						hr = _pD3D11Device->CreateTexture2D(&textureDesc, NULL, &_pTexture);
						if (SUCCEEDED(hr))
							_imgTmp.Create(textureDesc.Width, textureDesc.Height, 32);
						else LOG_ERROR("CreateTexture2D() Failed to create the texture! hr=0x%08x\n", (UINT)hr);
					}
					else LOG_ERROR("duplicateOutput() Failed to create the duplication output!\n");
				}
				else LOG_ERROR("QueryInterface() Failed to query the IDXGIOutput1 interface! hr=0x%08x\n", (UINT)hr);
			}
			else LOG_ERROR("D3D11 not supported!\n");
		}
		else LOG_ERROR("D3D11CreateDevice() returned error code 0x%08x!\n", (UINT)hr);

		pDxgiOutputSelected->Release();
		pDxgiAdapterSelected->Release();
	}
	else LOG_ERROR("WinScreenCapture_D3D11() Unable to find a pair <Adapters,Output> attached to desktop!\n");
}
//-------------------------------------------------------------------------------------------------

WinScreenCapture_D3D11::~WinScreenCapture_D3D11()
{
	// Clean up
	if (_pTexture) _pTexture->Release();
	if (_pDxgiOutputDuplication) _pDxgiOutputDuplication->Release();
	if (_pDxgiOutput) _pDxgiOutput->Release();
	if (_pD3D11Device) _pD3D11Device->Release();
	if (_pD3D11Context) _pD3D11Context->Release();
	if (!_imgTmp.IsNull()) _imgTmp.Destroy();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_D3D11::duplicateOutput()
{
	HRESULT hr = _pDxgiOutput->DuplicateOutput(_pD3D11Device, &_pDxgiOutputDuplication);
	if (SUCCEEDED(hr))
	{
		DXGI_OUTDUPL_DESC outputDuplDesc;
		ZeroMemory(&outputDuplDesc, sizeof(DXGI_OUTDUPL_DESC));
		_pDxgiOutputDuplication->GetDesc(&outputDuplDesc);
		if (outputDuplDesc.ModeDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM)
			return TRUE;

		_pDxgiOutputDuplication->Release();  _pDxgiOutputDuplication = nullptr;
		LOG_ERROR("IDXGIDuplicateOutput does not use RGBA (8 bit) format (format=%u)!\n", (UINT)outputDuplDesc.ModeDesc.Format);
	}
	else LOG_ERROR("DuplicateOutput() Failed to duplicate output from IDXGIOutput1! hr=0x%08x\n", (UINT)hr);
	return FALSE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_D3D11::getCurrentScreenSize(UINT &nSizeX, UINT &nSizeY) const
{
	DXGI_OUTPUT_DESC outputDesc;
	ZeroMemory(&outputDesc, sizeof(DXGI_OUTPUT_DESC));
	if (_pDxgiOutput && SUCCEEDED(_pDxgiOutput->GetDesc(&outputDesc)))
	{
		nSizeX = outputDesc.DesktopCoordinates.right;
		nSizeY = outputDesc.DesktopCoordinates.bottom;
		return TRUE;
	}
	return FALSE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

BOOL WinScreenCapture_D3D11::captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img)
{
	BOOL nRet = FALSE;
	if ((nSizeX > 0) && (nSizeY > 0) && !img.IsNull() && _pDxgiOutputDuplication)
	{
		DXGI_OUTDUPL_FRAME_INFO frameInfo;
		ZeroMemory(&frameInfo, sizeof(DXGI_OUTDUPL_FRAME_INFO));
		IDXGIResource *pDxgiResource = nullptr;
		HRESULT  hr = _pDxgiOutputDuplication->AcquireNextFrame(AQUIRE_TIMEOUT, &frameInfo, &pDxgiResource);
		if (SUCCEEDED(hr))
		{
			ID3D11Texture2D *pTextureTmp = nullptr;
			hr = pDxgiResource->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&pTextureTmp);
			if (SUCCEEDED(hr))
			{
				CImage *pWorkingImage = &img;
				if ((img.GetWidth() != _imgTmp.GetWidth()) || (img.GetHeight() != _imgTmp.GetHeight()))
					pWorkingImage = &_imgTmp;

				const int  nImgPitch = pWorkingImage->GetPitch();
				const int  nImgStride = 4 * pWorkingImage->GetWidth();
				uint8_t   *pDstData = (uint8_t*)pWorkingImage->GetBits();

				DXGI_MAPPED_RECT mappedRect;
				hr = _pDxgiOutputDuplication->MapDesktopSurface(&mappedRect);
				if (SUCCEEDED(hr))
				{
					const uint8_t *pSrcData = ((const uint8_t*) mappedRect.pBits) + nY0 * mappedRect.Pitch + nX0 * 4;
					for (unsigned int y = 0; y < nSizeY; ++y)
					{
						CopyMemory(pDstData, pSrcData, nImgStride);
						pDstData += nImgPitch;
						pSrcData += mappedRect.Pitch;
					}
					nRet = TRUE;
					hr = _pDxgiOutputDuplication->UnMapDesktopSurface();
					if (FAILED(hr))
						LOG_ERROR("UnMapDesktopSurface() Failed to unmap the desktop surface! hr=0x%08x\n", (UINT)hr);
				}
				else if (hr == DXGI_ERROR_UNSUPPORTED)
				{
					// According to the docs, when we receive this error we need to transfer the image to a staging surface and then lock the image by calling IDXGISurface::Map().
					_pD3D11Context->CopyResource(_pTexture, pTextureTmp);

					D3D11_MAPPED_SUBRESOURCE mappedSubResource;
					hr = _pD3D11Context->Map(_pTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
					if (SUCCEEDED(hr))
					{
						const uint8_t *pSrcData = ((const uint8_t*) mappedSubResource.pData) + nY0 * mappedSubResource.RowPitch + nX0 * 4;
						for (unsigned int y = 0; y < nSizeY; ++y)
						{
							CopyMemory(pDstData, pSrcData, nImgStride);
							pDstData += nImgPitch;
							pSrcData += mappedRect.Pitch;
						}
						nRet = TRUE;
						_pD3D11Context->Unmap(_pTexture, 0);
					}
					else LOG_ERROR("Map() Failed to map the staging texture! hr=0x%08x\n", (UINT)hr);
				}
				else LOG_ERROR("MapDesktopSurface() Failed to get access to the desktop surface! hr=0x%08x\n", (UINT)hr);

				if (nRet && (pWorkingImage != &img))
				{
					HDC hDCSrc = _imgTmp.GetDC();
					HDC hDCDst = img.GetDC();
					::SetStretchBltMode(hDCDst, HALFTONE);
					nRet = ::StretchBlt(hDCDst, 0, 0, img.GetWidth(), img.GetHeight(), hDCSrc, nX0, nY0, nSizeX, nSizeY, SRCCOPY | CAPTUREBLT);
					img.ReleaseDC();
					_imgTmp.ReleaseDC();
				}
				pTextureTmp->Release();
			}
			else LOG_ERROR("QueryInterface() Failed to query the ID3D11Texture2D interface on the IDXGIResource! hr=0x%08x\n", (UINT)hr);
			pDxgiResource->Release();
			hr = _pDxgiOutputDuplication->ReleaseFrame();
			if (FAILED(hr))
				LOG_ERROR("ReleaseFrame() Failed releasing the duplication frame! hr=0x%08x\n", (UINT)hr);
		}
		else LOG_ERROR("AcquireNextFrame() Failed aquiring the next frame! hr=0x%08x\n", (UINT)hr);
	}
	else LOG_ERROR("captureScreenRect() Invalid rect size (%ux%u), target image was not initialized, or DXGI OutputDuplication was not set!\n", nSizeX, nSizeY);
	return nRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
