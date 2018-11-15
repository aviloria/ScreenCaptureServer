/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES ANS MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINMEMSTREAM_H__
#define __WINMEMSTREAM_H__
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * IStream "fake" interface to dump to all streamed data into a memory buffer
 */
class WinMemStream : public IStream
{
public:
	WinMemStream(BYTE *pData, UINT nMaxSize, BOOL bDeleteOnDestroy);
	~WinMemStream();

	// IStream derived interfaces
	HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);
	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
	HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT STDMETHODCALLTYPE Revert();
	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
	HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag);
	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);

	// IUnknown derived interfaces
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &riid, void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);

	// ISequentialStream derived interfaces
	HRESULT STDMETHODCALLTYPE Read(void *, ULONG, ULONG *);
	HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten);

	// Own methods
	void reset();
	const BYTE  *getData() const;
	UINT getSize() const;

protected:
	UINT   _refCount;
	BYTE  *_pData;
	UINT   _nSize;
	UINT   _nMaxSize;
	BOOL   _bDeleteOnDestroy;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINMEMSTREAM_H__
//-------------------------------------------------------------------------------------------------
