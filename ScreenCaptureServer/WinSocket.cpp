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
#include <thread>
#include "WinSocket.h"
//-------------------------------------------------------------------------------------------------
// WSAGetLastError codes: https://docs.microsoft.com/es-es/windows/desktop/WinSock/windows-sockets-error-codes-2
//-------------------------------------------------------------------------------------------------
#if defined (_DEBUG)
#  define LOG(...)  ::fprintf(stdout, __VA_ARGS__)
#else
#  define LOG(...)
#endif
#define LOG_ERROR(...)  ::fprintf(stderr, __VA_ARGS__)
//-------------------------------------------------------------------------------------------------
#ifndef SD_SEND                                                                 
#  define  SD_SEND      0                                                       
#endif                                                                          
#ifndef SD_RECEIVE                                                              
#  define  SD_RECEIVE   1                                                       
#endif                                                                          
#ifndef SD_BOTH                                                                 
#  define  SD_BOTH      2                                                       
#endif  
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

class WSA_Helper
{
public:
	WSA_Helper()
	{
		// Initialize Winsock
		WSADATA wsaData;
		const int  nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nResult)
		{
			LOG_ERROR("WSA_Helper::WSAStartup failed with error: %d\n", nResult);
		}
		_isValid = !nResult;
	}
	~WSA_Helper() { WSACleanup(); }

	bool isValid() const { return _isValid; }
private:
	bool _isValid;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

WinSocket::WinSocket()
	: _sSocket(INVALID_SOCKET)
{
	// Initialize Windows Socket Layer...
	// + Creation is performed on the first WinSocket creation.
	// + Destruction is performed at program exit, only if a WinSocket was created.
	static const WSA_Helper __wsa;

	// Initialize Timeout
	_tvTimeout.tv_sec = 20;
	_tvTimeout.tv_usec = 0;
}
//-------------------------------------------------------------------------------------------------

WinSocket::~WinSocket()
{
	close();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

bool WinSocket::startServer(const char *strInterface, uint16_t nPort, FnConnection fnOnConnection, void *pParam, uint32_t nMaxConnections)
{
	// Host address resolution
	unsigned long  nAddress = INADDR_ANY;
	if (strInterface)
	{
		nAddress = inet_addr(strInterface);
		if (nAddress == INADDR_NONE)
		{
			hostent  *pInfoHost = gethostbyname(strInterface);
			if (!pInfoHost)
			{
				LOG_ERROR("WinSocket::gethostbyname() failed with error: %ld\n", WSAGetLastError());
				return false;
			}
			nAddress = *((unsigned long*)pInfoHost->h_addr_list[0]);
		}
	}

	// Create a SOCKET for connecting to server
	_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sSocket == INVALID_SOCKET)
	{
		LOG_ERROR("WinSocket::socket() failed with error: %ld\n", WSAGetLastError());
		return false;
	}

	// Setup the TCP listening socket
	_addrInfo.sin_family = AF_INET;
	_addrInfo.sin_addr.s_addr = nAddress;
	_addrInfo.sin_port = htons(nPort);
	if (bind(_sSocket, (sockaddr*)&_addrInfo, sizeof(_addrInfo)) != 0)
	{
		LOG_ERROR("WinSocket::bind() failed with error: %d\n", WSAGetLastError());
		close();
		return false;
	}

	if (listen(_sSocket, nMaxConnections) == SOCKET_ERROR)
	{
		LOG_ERROR("WinSocket::listen() failed with error: %d\n", WSAGetLastError());
		close();
		return false;
	}

	// Accept clients connections
	for (; ; )
	{
		WinSocket  *pClient = new WinSocket;
		pClient->_sSocket = accept(_sSocket, (sockaddr*)&pClient->_addrInfo, nullptr);
		if (pClient->_sSocket != INVALID_SOCKET)
		{
			std::thread threadProcess(fnOnConnection, pClient, pParam);
			threadProcess.detach();
		}
		else
		{
			LOG_ERROR("WinSocket::accept() failed with error: %d\n", WSAGetLastError());
		}
	}

	// Server termination
	close();
	return true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

bool WinSocket::open(const char *strAddress, uint16_t nPort)
{
	if (_sSocket != INVALID_SOCKET)
	{
		close();
	}

	// Host address resolution
	unsigned long  nAddress = inet_addr(strAddress);
	if (nAddress == INADDR_NONE)
	{
		hostent  *pInfoHost = gethostbyname(strAddress);
		if (!pInfoHost)
		{
			LOG_ERROR("WinSocket::gethostbyname() failed with error: %ld\n", WSAGetLastError());
			return false;
		}
		nAddress = *((unsigned long*)pInfoHost->h_addr_list[0]);
	}

	// Create a SOCKET for connecting to server
	_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sSocket == INVALID_SOCKET)
	{
		LOG_ERROR("WinSocket::socket() failed with error: %ld\n", WSAGetLastError());
		return false;
	}

	// Setup the TCP listening socket
	_addrInfo.sin_family = AF_INET;
	_addrInfo.sin_addr.s_addr = nAddress;
	_addrInfo.sin_port = htons(nPort);
	if (connect(_sSocket, (sockaddr*)&_addrInfo, sizeof(_addrInfo)) == SOCKET_ERROR)
	{
		LOG_ERROR("WinSocket::connect() failed with error: %d\n", WSAGetLastError());
		close();
		return false;
	}

	return true;
}
//-------------------------------------------------------------------------------------------------

bool WinSocket::close()
{
	if (_sSocket != INVALID_SOCKET)
	{
		shutdown(_sSocket, SD_BOTH);
		if (closesocket(_sSocket) == SOCKET_ERROR)
		{
			LOG_ERROR("WinSocket::closesocket() failed with error: %d\n", WSAGetLastError());
			return false;
		}
		_sSocket = INVALID_SOCKET;
	}
	return true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void WinSocket::setTimeOut(uint32_t nTimeOutInMillisecs)
{
	// Configure timeout
	_tvTimeout.tv_sec = nTimeOutInMillisecs / 1000;
	_tvTimeout.tv_usec = (nTimeOutInMillisecs % 1000) * 1000;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

uint32_t WinSocket::read(uint8_t *pBuffer, uint32_t nBufferSize)
{
	if (_sSocket == INVALID_SOCKET)
	{
		return 0;
	}

	// Initialize file descriptor sets
	fd_set  readset;
	FD_ZERO(&readset);
	FD_SET(_sSocket, &readset);

	// Wait for valid data
	int  nRet = 0;
	switch (select((int)_sSocket + 1, &readset, nullptr, nullptr, &_tvTimeout))
	{
	case -1: // Error
		LOG_ERROR("WinSocket::select() failed with error: %d\n", WSAGetLastError());
		break;
	case 0: // Timeout
		LOG_ERROR("WinSocket::select() Timeout!\n");
		break;
	default:
		if (FD_ISSET(_sSocket, &readset))
		{
			nRet = recv(_sSocket, (char*)pBuffer, nBufferSize, 0);
		}
		if (nRet == SOCKET_ERROR)
		{
			LOG_ERROR("WinSocket::recv() failed with error: %d\n", WSAGetLastError());
		}
		if (nRet < 0)
		{
			nRet = 0;
		}
		break;
	}
	return (uint32_t)nRet;
}
//-------------------------------------------------------------------------------------------------

uint32_t WinSocket::write(const uint8_t *pBuffer, uint32_t nBufferSize)
{
	if (_sSocket == INVALID_SOCKET)
	{
		return 0;
	}

	// Initialize file descriptor sets
	fd_set  writeset;
	FD_ZERO(&writeset);
	FD_SET(_sSocket, &writeset);

	// Wait for valid data
	int  nRet = 0;
	switch (::select((int)_sSocket + 1, nullptr, &writeset, nullptr, &_tvTimeout))
	{
	case -1: // Error
		LOG_ERROR("WinSocket::select() failed with error: %d\n", WSAGetLastError());
		break;
	case 0: // Timeout
		LOG_ERROR("WinSocket::select() Timeout!\n");
		break;
	default:
		if (FD_ISSET(_sSocket, &writeset))
		{
			nRet = send(_sSocket, (const char*)pBuffer, nBufferSize, 0);
		}
		if (nRet == SOCKET_ERROR)
		{
			LOG_ERROR("WinSocket::write() failed with error: %d\n", WSAGetLastError());
		}
		if (nRet < 0)
		{
			nRet = 0;
		}
		break;
	}
	return (uint32_t)nRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
