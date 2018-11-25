/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSCREENCAPTURE_D3D9_H__
#define __WINSCREENCAPTURE_D3D9_H__
//-------------------------------------------------------------------------------------------------
#include "IWinScreenCapture.h"
#include <d3d9.h>
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * Screen capture class for capturing using Direct3D 9
 */
class WinScreenCapture_D3D9 : public IWinScreenCapture
{
public:
	WinScreenCapture_D3D9();
	~WinScreenCapture_D3D9();

	BOOL captureScreenRect(UINT nX0, UINT nY0, UINT nSizeX, UINT nSizeY, CImage &img);

private:
	IDirect3D9        *_pD3D;
	IDirect3DDevice9  *_pDevice;
	IDirect3DSurface9 *_pSurface;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSCREENCAPTURE_D3D9_H__
//-------------------------------------------------------------------------------------------------
