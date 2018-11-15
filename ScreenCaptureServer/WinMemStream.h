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
	HRESULT Clone(IStream **ppstm);
	HRESULT Commit(DWORD grfCommitFlags);
	HRESULT CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
	HRESULT LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT Revert();
	HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
	HRESULT SetSize(ULARGE_INTEGER libNewSize);
	HRESULT Stat(STATSTG *pstatstg, DWORD grfStatFlag);
	HRESULT UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);

	// IUnknown derived interfaces
	HRESULT QueryInterface(const IID &riid, void **ppvObject);
	ULONG AddRef(void);
	ULONG Release(void);

	// ISequentialStream derived interfaces
	HRESULT Read(void *, ULONG, ULONG *);
	HRESULT Write(const void *pv, ULONG cb, ULONG *pcbWritten);

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
