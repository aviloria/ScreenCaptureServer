/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES ANS MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "WinMemStream.h"
//-------------------------------------------------------------------------------------------------
#if defined (_DEBUG)
#  define LOG(...)  ::fprintf(stdout, __VA_ARGS__)
#else
#  define LOG(...)
#endif
#define LOG_ERROR(...)  ::fprintf(stderr, __VA_ARGS__)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinMemStream::WinMemStream(BYTE *pData, UINT nMaxSize, BOOL bDeleteOnDestroy)
	: _refCount(1)
	, _pData(pData)
	, _nSize(0)
	, _nMaxSize(nMaxSize)
	, _bDeleteOnDestroy(bDeleteOnDestroy)
{
}
//-------------------------------------------------------------------------------------------------

WinMemStream::~WinMemStream()
{
	if (_pData && _bDeleteOnDestroy)
	{
		delete[] _pData;
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Clone(IStream **ppstm)
{
	LOG("WinMemStream::Clone()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Commit(DWORD grfCommitFlags)
{
	LOG("WinMemStream::Commit()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	LOG("WinMemStream::CopyTo()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	LOG("WinMemStream::LockRegion()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Revert()
{
	LOG("WinMemStream::Revert()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	LOG("WinMemStream::Seek()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::SetSize(ULARGE_INTEGER libNewSize)
{
	LOG("WinMemStream::SetSize()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	LOG("WinMemStream::Stat()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	LOG("WinMemStream::UnlockRegion()\n");
	return S_OK;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::QueryInterface(const IID &riid, void **ppvObject)
{
	return E_NOINTERFACE;/*S_OK;*/
}
//-------------------------------------------------------------------------------------------------

ULONG WinMemStream::AddRef(void)
{
	LOG("WinMemStream::AddRef()\n");
	return ++_refCount;
}
//-------------------------------------------------------------------------------------------------

ULONG WinMemStream::Release(void)
{
	LOG("WinMemStream::Release()\n");
	return --_refCount;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Read(void *pv, ULONG cb, ULONG *pcbReaded)
{
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------

HRESULT WinMemStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	ULONG nWritten = 0;
	LOG("WinMemStream::Write()\n");
	if (pv)
	{
		if (_pData && cb)
		{
			nWritten = _nMaxSize - _nSize;
			if (nWritten > cb)
			{
				nWritten = cb;
			}
			memcpy (_pData + _nSize, pv, nWritten);
			_nSize += nWritten;
		}
		if (pcbWritten)
		{
			*pcbWritten = nWritten;
		}
		return S_OK;
	}
	return E_FAIL;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void WinMemStream::reset()
{
	_nSize = 0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

const BYTE *WinMemStream::getData() const
{
	return _pData;
}
//-------------------------------------------------------------------------------------------------

UINT WinMemStream::getSize() const
{
	return _nSize;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
