/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <string.h>
#include <thread>
#include "WinSocket.h"
#include "WinMemStream.h"
#include "WinScreenCapture_GDI.h"
#include "WinScreenCapture_GDI+.h"
#include "WinScreenCapture_D3D9.h"
#include "WinScreenCapture_D3D11.h"
//-------------------------------------------------------------------------------------------------
#pragma warning (disable : 4996) // Remove ::sprintf() security warnings
//-------------------------------------------------------------------------------------------------
#define MAX_LINE_SIZE   1024
#define BPS               32 //24 // Bits-per-pixel value to be used in the image
#define CRLF          "\r\n"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

enum class Mode { Unknown, ScreenShot, Video, HealthCheck };
enum class Capturer { Unknown, None, GDI, GDIplus, D3D9, D3D11 };
//-------------------------------------------------------------------------------------------------

struct RequestArguments
{
	uint32_t nWidth;     // Destination image width
	uint32_t nHeight;    // Destination image height
	uint32_t nX0;        // Snapshot offset in X-axis
	uint32_t nY0;        // Snapshot offset in Y-axis
	uint32_t nCX;        // Snapshot width
	uint32_t nCY;        // Snapshot height
	uint32_t nFPS;       // Frame rate (video only)
	Capturer eCapturer;  // Capturer type to be used
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
	Mode mode = Mode::Unknown;
	char strLine[MAX_LINE_SIZE + 1];

	// Get request info
	const char *strArguments = nullptr;
	if (getLine(pSocket, strLine, MAX_LINE_SIZE))
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
		else if (strstr(strLine, "GET /healthCheck") == strLine)
		{
			arguments.eCapturer = Capturer::None;
			mode = Mode::HealthCheck;
		}
	}
	// Process request arguments
	if (strArguments && (strArguments[0] != '\0'))
	{
		const char *strAux = strstr(strArguments, "width=");
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
		strAux = strstr(strArguments, "cap=GDI");
		if (strAux) arguments.eCapturer = Capturer::GDI;
		strAux = strstr(strArguments, "cap=GDI+");
		if (strAux) arguments.eCapturer = Capturer::GDIplus;
		strAux = strstr(strArguments, "cap=D3D9");
		if (strAux) arguments.eCapturer = Capturer::D3D9;
		strAux = strstr(strArguments, "cap=D3D11");
		if (strAux) arguments.eCapturer = Capturer::D3D11;
	}

	// Get Host info
	if (getLine(pSocket, strLine, MAX_LINE_SIZE))
	{
		if (strstr(strLine, "Host: ") == strLine)
		{
			::fprintf(stdout, "Connection from %s\n", strLine);
		}
	}

	// Consume remaining header lines
	while (getLine(pSocket, strLine, MAX_LINE_SIZE) && (strLine[0] != '\0'));

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
	pSocket->write((const uint8_t*) strBadRequest, (uint32_t)strlen(strBadRequest));
}
//-------------------------------------------------------------------------------------------------

void sendHttpInternalError(WinSocket *pSocket)
{
	static const char *strBadRequest =
		"HTTP/1.1 500 Internal Server Error" CRLF
		"server: screencap" CRLF
		"Content-Type: text/html" CRLF
		"Content-Length: 190" CRLF
		"Connection: close" CRLF
		CRLF
		"<html>" CRLF
		"<head><title>500 Internal Server Error</title></head>" CRLF
		"<body bgcolor = \"white\">" CRLF
		"<center><h1>500 Internal Server Error</h1></center>" CRLF
		"<hr><center>screencap</center>" CRLF
		"</body>" CRLF
		"</html>";
	pSocket->write((const uint8_t*) strBadRequest, (uint32_t)strlen(strBadRequest));
}
//-------------------------------------------------------------------------------------------------

void sendHttpOK(WinSocket *pSocket, const char *strMIME, const uint8_t *pData, uint32_t nSize)
{
	char strHeader[MAX_LINE_SIZE];
	if (nSize)
	{
		::sprintf(strHeader,
			"HTTP/1.1 200 OK" CRLF
			"server: screencap" CRLF
			"Content-Type: %s" CRLF
			"Content-Length: %u" CRLF
			"Connection: keep-alive" CRLF
			"Allow: GET, OPTIONS" CRLF
			"Access-Control-Allow-Origin: *" CRLF
			"Access-Control-Allow-Methods: GET, OPTIONS" CRLF
			"Access-Control-Allow-Headers: Content-Type" CRLF
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
			"Allow: GET, OPTIONS" CRLF
			"Access-Control-Allow-Origin: *" CRLF
			"Access-Control-Allow-Methods: GET, OPTIONS" CRLF
			"Access-Control-Allow-Headers: Content-Type" CRLF
			CRLF,
			strMIME);
	}
	pSocket->write((const uint8_t*) strHeader, (uint32_t)strlen(strHeader));
	pSocket->write(pData, nSize);
}
//-------------------------------------------------------------------------------------------------

bool sendHttpMultiPart(WinSocket *pSocket, const char *strBoundaryName, const char *strMIME, const uint8_t *pData, uint32_t nSize)
{
	char strMultipart[MAX_LINE_SIZE];
	::sprintf(strMultipart,
		"--%s" CRLF
		"Content-Type: %s" CRLF
		"Content-Length: %u" CRLF
		CRLF,
		strBoundaryName, strMIME, nSize);
	return pSocket->write((const uint8_t*) strMultipart, (uint32_t)strlen(strMultipart)) &&
			(pSocket->write(pData, nSize) == nSize) &&
			(pSocket->write((const uint8_t*) CRLF, 2) == 2);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

IWinScreenCapture *checkArguments(RequestArguments &arguments)
{
	const uint32_t nSizeX = ::GetSystemMetrics(SM_CXSCREEN);
	const uint32_t nSizeY = ::GetSystemMetrics(SM_CYSCREEN);
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

	// Set capturer
	IWinScreenCapture *pRet = nullptr;
	switch (arguments.eCapturer)
	{
		case Capturer::None:
			break;
		case Capturer::Unknown: // Fallthrough
		case Capturer::GDI:
			::fprintf(stdout, "Using GDI capturer...\n");
			pRet = new WinScreenCapture_GDI;
			break;
		case Capturer::GDIplus:
			::fprintf(stdout, "Using GDI+ capturer...\n");
			pRet = new WinScreenCapture_GDIplus;
			break;
		case Capturer::D3D9:
			::fprintf(stdout, "Using D3D9 capturer...\n");
			pRet = new WinScreenCapture_D3D9;
			break;
		case Capturer::D3D11:
			::fprintf(stdout, "Using D3D11 capturer...\n");
			pRet = new WinScreenCapture_D3D11;
			break;
	}
	return pRet;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void onHttpConnection(WinSocket *pSocket, void * /*pParam*/)
{
	RequestArguments arguments;
	arguments.nWidth    = 0;
	arguments.nHeight   = 0;
	arguments.nX0       = 0;
	arguments.nY0       = 0;
	arguments.nCX       = 0;
	arguments.nCY       = 0;
	arguments.nFPS      = 0;
	arguments.eCapturer = Capturer::Unknown;

	// Process the request
	switch (processHttpRequest(pSocket, arguments))
	{
		case Mode::Unknown:
			sendHttpBadRequest(pSocket);
			break;

		case Mode::HealthCheck:
		{
			IWinScreenCapture *pScreenCapture = checkArguments(arguments);
			char strJson[256];
			::sprintf(strJson, "{ \"ip\" : \"%s\", \"hostname\" : \"%s\", \"width\" : %u, \"height\" : %u }",
				pSocket->getIpAddress(WinSocket::getHostName()), WinSocket::getHostName(), arguments.nWidth, arguments.nHeight);
			sendHttpOK(pSocket, "application/json", (const uint8_t*)strJson, (uint32_t)strlen(strJson));
			if (pScreenCapture) delete pScreenCapture;
			break;
		}

		case Mode::ScreenShot:
		{
			IWinScreenCapture *pScreenCapture = checkArguments(arguments);
			if (pScreenCapture)
			{
				const uint32_t nBufferSize = ((arguments.nWidth * arguments.nHeight * BPS) / 8) + 100;
				uint8_t *pBuffer = new uint8_t[nBufferSize];

				CImage img;
				img.Create(arguments.nWidth, arguments.nHeight, BPS);
				if (pScreenCapture->captureScreenRect(arguments.nX0, arguments.nY0, arguments.nCX, arguments.nCY, img))
				{
					WinMemStream stream(pBuffer, nBufferSize, false);
					img.Save(&stream, ImageFormatJPEG);
					sendHttpOK(pSocket, "image/jpeg", stream.getData(), stream.getSize());
				}
				else sendHttpInternalError(pSocket);

				delete[] pBuffer;
				delete pScreenCapture;
			}
			else sendHttpInternalError(pSocket);
			break;
		}

		case Mode::Video:
		{
			IWinScreenCapture *pScreenCapture = checkArguments(arguments);
			if (pScreenCapture)
			{
				const uint32_t nBufferSize = ((arguments.nWidth * arguments.nHeight * BPS) / 8) + 100;
				uint8_t *pBuffer = new uint8_t[nBufferSize];
				CImage img;
				img.Create(arguments.nWidth, arguments.nHeight, BPS);

				const double dExpectedTimeBetweenFrames = 1.0 / ((double)arguments.nFPS);
				double dAvgTimeBetweenFrames = dExpectedTimeBetweenFrames;
				std::chrono::high_resolution_clock::time_point tpBegin = std::chrono::high_resolution_clock::now();
				if (pScreenCapture->captureScreenRect(arguments.nX0, arguments.nY0, arguments.nCX, arguments.nCY, img))
				{
					sendHttpOK(pSocket, "multipart/x-mixed-replace; boundary=\"SCREENCAP_MJPEG\"", nullptr, 0);
					for (bool bContinue = true; bContinue; )
					{
						WinMemStream stream(pBuffer, nBufferSize, false);
						img.Save(&stream, ImageFormatJPEG);
						bContinue = sendHttpMultiPart(pSocket, "SCREENCAP_MJPEG", "image/jpeg", stream.getData(), stream.getSize()) &&
							pScreenCapture->captureScreenRect(arguments.nX0, arguments.nY0, arguments.nCX, arguments.nCY, img);

						// Compute elapsed time and perform a sleep to meet FPS requirements (if needed)
						if (bContinue)
						{
							const std::chrono::high_resolution_clock::time_point tpEnd = std::chrono::high_resolution_clock::now();
							const std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(tpEnd - tpBegin);
							dAvgTimeBetweenFrames += dExpectedTimeBetweenFrames * (timeSpan.count() - dAvgTimeBetweenFrames); // 1 second estimation window
							if (dExpectedTimeBetweenFrames > dAvgTimeBetweenFrames)
								std::this_thread::sleep_for(std::chrono::duration<double>(dExpectedTimeBetweenFrames - dAvgTimeBetweenFrames));
							tpBegin = std::chrono::high_resolution_clock::now();
						}
					}
				}
				else sendHttpInternalError(pSocket);

				delete[] pBuffer;
				delete pScreenCapture;
			}
			else sendHttpInternalError(pSocket);
			break;
		}
	}
	
	// Free up resources
	pSocket->close();
	delete pSocket;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	::fprintf(stdout, "ScreenCaptureServer v1.0.2. By @aviloria\n");
	
	// Parameter validation
	const char *strInterface = nullptr;
	uint16_t    nPort = 8080;
	uint16_t    nMaxConnections = 10;
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
