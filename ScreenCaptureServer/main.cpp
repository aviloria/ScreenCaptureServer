/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES ANS MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <string.h>
#include <thread>
#include "WinSocket.h"
#include "WinMemStream.h"
#include "WinScreenCapture.h"
//-------------------------------------------------------------------------------------------------
#pragma warning (disable : 4996) // Remove ::sprintf() security warnings
//-------------------------------------------------------------------------------------------------
#define CRLF  "\r\n"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

enum class Mode { Unknown, ScreenShot, Video };
//-------------------------------------------------------------------------------------------------

struct RequestArguments
{
	uint32_t nWidth;	// Destination image width
	uint32_t nHeight;   // Destination image height
	uint32_t nX0;       // Snapshot offset in X-axis
	uint32_t nY0;       // Snapshot offset in Y-axis
	uint32_t nCX;       // Snapshot width
	uint32_t nCY;       // Snapshot height
	uint32_t nFPS;      // Frame rate (video only)
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

bool getLine(WinSocket *pSocket, char *strLine, uint32_t nLineSize)
{
	bool bRet = false;
	while (nLineSize && pSocket->read((uint8_t*)strLine, 1) && (*strLine != '\n'))
	{
		if (*strLine != '\r')
			{ ++strLine;  --nLineSize; }
		bRet = true;
	}
	bRet |= (*strLine == '\n');
	*strLine = '\0';
	return bRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Mode processHttpRequest(WinSocket *pSocket, RequestArguments &arguments)
{
	Mode  mode = Mode::Unknown;
	char  strLine[1025];

	// Get request info
	const char *strArguments = nullptr;
	if (getLine(pSocket, strLine, 1024))
	{
		if (strstr(strLine, "GET /getImage") == strLine)
		{
			strArguments = strLine + 13;
			mode = Mode::ScreenShot;
		}
		else if (strstr(strLine, "GET /getVideo") == strLine)
		{
			strArguments = strLine + 13;
			mode = Mode::Video;
		}
		else
		{
			mode = Mode::Unknown;
		}
	}
	// Process request arguments
	if (strArguments && (strArguments[0] != '\0'))
	{
		const char  *strAux = strstr(strArguments, "width=");
		if (strAux) arguments.nWidth = atoi(strAux + 6);
		strAux = strstr(strArguments, "height=");
		if (strAux) arguments.nHeight = atoi(strAux + 7);
		strAux = strstr(strArguments, "x0=");
		if (strAux) arguments.nX0 = atoi(strAux + 3);
		strAux = strstr(strArguments, "y0=");
		if (strAux) arguments.nY0 = atoi(strAux + 3);
		strAux = strstr(strArguments, "cx=");
		if (strAux) arguments.nCX = atoi(strAux + 3);
		strAux = strstr(strArguments, "cy=");
		if (strAux) arguments.nCY = atoi(strAux + 3);
		strAux = strstr(strArguments, "fps=");
		if (strAux) arguments.nFPS = atoi(strAux + 4);
	}

	// Get Host info
	if (getLine(pSocket, strLine, 1024))
	{
		if (strstr(strLine, "Host: ") == strLine)
		{
			::fprintf(stdout, "Connection from %s\n", strLine);
		}
	}

	// Consume remaining header lines
	while (getLine(pSocket, strLine, 1024) && (strLine[0] != '\0'));

	return mode;
}
//-------------------------------------------------------------------------------------------------

void sendHttpBadRequest(WinSocket *pSocket)
{
	static const char *strBadRequest =
		"HTTP/1.1 400 Invalid HTTP Request" CRLF
		"server: screencap" CRLF
		"Content-Type: text/html" CRLF
		"Content-Length: 170" CRLF
		"Connection: close" CRLF
		CRLF
		"<html>" CRLF
		"<head><title>400 Bad Request</title></head>" CRLF
		"<body bgcolor = \"white\">" CRLF
		"<center><h1>400 Bad Request</h1></center>" CRLF
		"<hr><center>screencap</center>" CRLF
		"</body>" CRLF
		"</html>";
	pSocket->write((const uint8_t*) strBadRequest, (uint32_t) strlen(strBadRequest));
}
//-------------------------------------------------------------------------------------------------

void sendHttpOK(WinSocket *pSocket, const char *strMIME, const uint8_t *pData, uint32_t nSize)
{
	char strHeader[256];
	if (nSize)
	{
		::sprintf(strHeader,
			"HTTP/1.1 200 OK" CRLF
			"server: screencap" CRLF
			"Content-Type: %s" CRLF
			"Content-Length: %u" CRLF
			"Connection: keep-alive" CRLF
			CRLF,
			strMIME, nSize);
	}
	else
	{
		::sprintf(strHeader,
			"HTTP/1.1 200 OK" CRLF
			"server: screencap" CRLF
			"Content-Type: %s" CRLF
			"Connection: keep-alive" CRLF
			CRLF,
			strMIME);
	}
	pSocket->write((const uint8_t*) strHeader, (uint32_t) strlen(strHeader));
	pSocket->write(pData, nSize);
}
//-------------------------------------------------------------------------------------------------

bool sendHttpMultiPart(WinSocket *pSocket, const char *strBoundaryName, const char *strMIME, const uint8_t *pData, uint32_t nSize)
{
	char strMultipart[256];
	::sprintf(strMultipart,
		"--%s" CRLF
		"Content-Type: %s" CRLF
		"Content-Length: %u" CRLF
		CRLF,
		strBoundaryName, strMIME, nSize);
	return pSocket->write((const uint8_t*) strMultipart, (uint32_t) strlen(strMultipart)) &&
			(pSocket->write(pData, nSize) == nSize) &&
			(pSocket->write((const uint8_t*) CRLF, 2) == 2);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void checkArguments(RequestArguments &arguments)
{
	const uint32_t  nSizeX = ::GetSystemMetrics(SM_CXSCREEN);
	const uint32_t  nSizeY = ::GetSystemMetrics(SM_CYSCREEN);
	if (!arguments.nCX || !arguments.nCY ||
		(arguments.nX0 + arguments.nCX > nSizeX) ||
		(arguments.nY0 + arguments.nCY > nSizeY))
	{
		arguments.nX0 = 0;       arguments.nY0 = 0;
		arguments.nCX = nSizeX;  arguments.nCY = nSizeY;
	}
	if (arguments.nWidth && !arguments.nHeight)
	{
		arguments.nHeight = (arguments.nWidth * arguments.nCY) / arguments.nCX;
	}
	if (!arguments.nWidth && arguments.nHeight)
	{
		arguments.nWidth = (arguments.nHeight * arguments.nCX) / arguments.nCY;
	}
	if (!arguments.nWidth && !arguments.nHeight)
	{
		arguments.nWidth  = arguments.nCX;
		arguments.nHeight = arguments.nCY;
	}
	if (!arguments.nFPS)
	{
		arguments.nFPS = 25;
	}

	// Take care about resizing aspect ratio
	uint32_t nDstWidth = arguments.nCX;
	uint32_t nDstHeight = arguments.nCY;
	if (nDstWidth > arguments.nWidth)
	{
		nDstWidth = arguments.nWidth;
		nDstHeight = (nDstWidth * arguments.nCY) / arguments.nCX;
	}
	if (nDstHeight > arguments.nHeight)
	{
		nDstHeight = arguments.nHeight;
		nDstWidth = (nDstHeight * arguments.nCX) / arguments.nCY;
	}

	// Set destination size
	arguments.nWidth  = nDstWidth;
	arguments.nHeight = nDstHeight;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void onHttpConnection(WinSocket *pSocket, void * /*pParam*/)
{
	RequestArguments arguments;
	arguments.nWidth = 0;
	arguments.nHeight = 0;
	arguments.nX0 = 0;
	arguments.nY0 = 0;
	arguments.nCX = 0;
	arguments.nCY = 0;
	arguments.nFPS = 25;

	// Process HTTP request
	const Mode mode = processHttpRequest(pSocket, arguments);
	if (mode == Mode::Unknown)
	{
		sendHttpBadRequest(pSocket);
		pSocket->close();
		return;
	}
	checkArguments (arguments);

	// Process the request
	const uint32_t  nBufferSize = arguments.nWidth * arguments.nHeight * 3 + 100;
	uint8_t *pBuffer = new uint8_t[nBufferSize];

	CImage img;
	img.Create(arguments.nWidth, arguments.nHeight, 24);
	switch (mode)
	{
		case Mode::ScreenShot:
		{
			WinMemStream stream(pBuffer, nBufferSize, false);
			WinScreenCapture::captureScreenRect(arguments.nX0, arguments.nY0, arguments.nCX, arguments.nCY, img);
			img.Save(&stream, ImageFormatJPEG);

			sendHttpOK(pSocket, "image/jpeg", stream.getData(), stream.getSize());
			pSocket->close();
			break;
		}

		case Mode::Video:
		{
			const std::chrono::milliseconds delay(1000 / arguments.nFPS);
			sendHttpOK(pSocket, "multipart/x-mixed-replace; boundary=\"SCREENCAP_MJPEG\"", nullptr, 0);
			for (bool bContinue = true; bContinue; )
			{
				WinMemStream stream(pBuffer, nBufferSize, false);
				WinScreenCapture::captureScreenRect(arguments.nX0, arguments.nY0, arguments.nCX, arguments.nCY, img);
				img.Save(&stream, ImageFormatJPEG);

				bContinue = sendHttpMultiPart(pSocket, "SCREENCAP_MJPEG", "image/jpeg", stream.getData(), stream.getSize());
				std::this_thread::sleep_for(delay);
			}
			pSocket->close();
			break;
		}
	}
	delete[] pBuffer;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	// Parameter validation
	const char  *strInterface = nullptr;
	uint16_t     nPort = 8080;
	uint16_t     nMaxConnections = 10;
	for (int n = 1; n < argc; ++n)
	{
		if (strstr(argv[n], "-i:") == argv[n])
		{
			strInterface = argv[n] + 3;
		}
		else if (strstr(argv[n], "-p:") == argv[n])
		{
			nPort = atoi(argv[n] + 3);
		}
		else if (strstr(argv[n], "-c:") == argv[n])
		{
			nMaxConnections = atoi(argv[n] + 3);
		}
		else
		{
			::fprintf(stdout, "Syntax:\n");
			::fprintf(stdout, "\t%s [options]\n", argv[0]);
			::fprintf(stdout, "where 'options' is a combination of:\n");
			::fprintf(stdout, "-i:<strInterface>      Server interface for accepting connections\n");
			::fprintf(stdout, "                       Default value: ANY\n");
			::fprintf(stdout, "-p:<nPort>             Server port number for accepting connections\n");
			::fprintf(stdout, "                       Default value: 8080\n");
			::fprintf(stdout, "-c:<nMaxConnections>   Maximum simultanous connections\n");
			::fprintf(stdout, "                       Default value: 10\n");
			return 1;
		}
	}
	::fprintf(stdout, "Starting server...\n");
	::fprintf(stdout, "+ Interface: %s\n", strInterface ? strInterface : "ANY");
	::fprintf(stdout, "+ Port     : %u\n", nPort);
	::fprintf(stdout, "+ Sim.conn.: %u\n", nMaxConnections);

	// Start server
	WinSocket server;
	server.startServer(strInterface, nPort, onHttpConnection, nullptr, nMaxConnections);
	return 0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
