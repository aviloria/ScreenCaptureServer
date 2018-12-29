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
#include <chrono>
#include "WinSocket.h"
#include "WinMemStream.h"
#include "WinScreenCaptureHelper.h"
//-------------------------------------------------------------------------------------------------
#pragma warning (disable : 4996) // Remove ::sprintf() security warnings
//-------------------------------------------------------------------------------------------------
#define MAX_LINE_SIZE   1024
#define BPS               32 //24 // Bits-per-pixel value to be used in the image
#define CRLF          "\r\n"
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

enum class Mode { Unknown, ScreenShot, Video, HealthCheck };
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

Mode processHttpRequest(WinSocket *pSocket, WinScreenCaptureHelper::Settings &settings)
{
	Mode mode = Mode::Unknown;
	char strLine[MAX_LINE_SIZE + 1];

	// Get request info
	const char *strArguments = nullptr;
	if (getLine(pSocket, strLine, MAX_LINE_SIZE))
	{
		if (::strstr(strLine, "GET /getImage") == strLine)
		{
			strArguments = strLine + 13;
			mode = Mode::ScreenShot;
		}
		else if (::strstr(strLine, "GET /getVideo") == strLine)
		{
			strArguments = strLine + 13;
			mode = Mode::Video;
		}
		else if (::strstr(strLine, "GET /healthCheck") == strLine)
		{
			settings.eCapturer = WinScreenCaptureHelper::Capturer::None;
			mode = Mode::HealthCheck;
		}
	}
	// Process request settings
	if (strArguments && (strArguments[0] != '\0'))
	{
		const char *strAux = ::strstr(strArguments, "width=");
		if (strAux) settings.nWidth = atoi(strAux + 6);
		strAux = ::strstr(strArguments, "height=");
		if (strAux) settings.nHeight = atoi(strAux + 7);
		strAux = ::strstr(strArguments, "x0=");
		if (strAux) settings.nX0 = atoi(strAux + 3);
		strAux = ::strstr(strArguments, "y0=");
		if (strAux) settings.nY0 = atoi(strAux + 3);
		strAux = ::strstr(strArguments, "cx=");
		if (strAux) settings.nCX = atoi(strAux + 3);
		strAux = ::strstr(strArguments, "cy=");
		if (strAux) settings.nCY = atoi(strAux + 3);
		strAux = ::strstr(strArguments, "fps=");
		if (strAux) settings.nFPS = atoi(strAux + 4);
		strAux = ::strstr(strArguments, "cap=GDI");
		if (strAux) settings.eCapturer = WinScreenCaptureHelper::Capturer::GDI;
		strAux = ::strstr(strArguments, "cap=GDI+");
		if (strAux) settings.eCapturer = WinScreenCaptureHelper::Capturer::GDIplus;
		strAux = ::strstr(strArguments, "cap=D3D9");
		if (strAux) settings.eCapturer = WinScreenCaptureHelper::Capturer::D3D9;
		strAux = ::strstr(strArguments, "cap=D3D11");
		if (strAux) settings.eCapturer = WinScreenCaptureHelper::Capturer::D3D11;
		strAux = ::strstr(strArguments, "cap=RDP");
		if (strAux) settings.eCapturer = WinScreenCaptureHelper::Capturer::RDP;
		strAux = ::strstr(strArguments, "dev=");
		if (strAux)
		{
			uint32_t nPos = 0;
			strAux += 4;
			while ((nPos < sizeof(settings.strDevice)) && (strAux[nPos] != ' ') && (strAux[nPos] != '&') && (strAux[nPos] != '\0'))
			{
				settings.strDevice[nPos] = strAux[nPos];
				nPos;
			}
			settings.strDevice[nPos] = '\0';
		}
	}

	// Get Host info
	if (getLine(pSocket, strLine, MAX_LINE_SIZE))
	{
		if (::strstr(strLine, "Host: ") == strLine)
		{
			LOG_INFO("Connection from %s\n", strLine);
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

void onUnkownCmd(WinSocket *pSocket, WinScreenCaptureHelper::Settings &/*settings*/, const WinScreenCaptureHelper * /*pScreenCapturerHelper*/)
{
	sendHttpBadRequest(pSocket);
}
//-------------------------------------------------------------------------------------------------

void onHealthCheckCmd(WinSocket *pSocket, WinScreenCaptureHelper::Settings &settings, const WinScreenCaptureHelper *pScreenCapturerHelper)
{
	IWinScreenCapture *pScreenCapture = pScreenCapturerHelper->checkSettings(settings);
	char strJson[4096];
	::sprintf(strJson, "{ \"ip\" : \"%s\", \"hostname\" : \"%s\", \"width\" : %u, \"height\" : %u, \"caps\" : %s, \"devices\" : %s }",
		pSocket->getIpAddress(WinSocket::getHostName()), WinSocket::getHostName(), settings.nWidth, settings.nHeight,
		pScreenCapturerHelper->getCapabilitiesString().c_str(), pScreenCapturerHelper->getDisplayDevicesString().c_str());
	sendHttpOK(pSocket, "application/json", (const uint8_t*)strJson, (uint32_t)strlen(strJson));
	if (pScreenCapture) delete pScreenCapture;
}
//-------------------------------------------------------------------------------------------------

void onScreenShotCmd(WinSocket *pSocket, WinScreenCaptureHelper::Settings &settings, const WinScreenCaptureHelper *pScreenCapturerHelper)
{
	IWinScreenCapture *pScreenCapture = pScreenCapturerHelper->checkSettings(settings);
	if (pScreenCapture)
	{
		const uint32_t nBufferSize = ((settings.nWidth * settings.nHeight * BPS) / 8) + 100;
		uint8_t *pBuffer = (uint8_t*)::malloc(nBufferSize);
		if (pBuffer)
		{
			CImage img;
			if (img.Create(settings.nWidth, settings.nHeight, BPS))
			{
				if (pScreenCapture->captureScreenRect(settings.nX0, settings.nY0, settings.nCX, settings.nCY, img))
				{
					WinMemStream stream(pBuffer, nBufferSize, false);
					img.Save(&stream, ImageFormatJPEG);
					sendHttpOK(pSocket, "image/jpeg", stream.getData(), stream.getSize());
				}
				else sendHttpInternalError(pSocket);
			}
			else sendHttpInternalError(pSocket);
			::free(pBuffer);
		}
		else sendHttpInternalError(pSocket);
		delete pScreenCapture;
	}
	else sendHttpInternalError(pSocket);
}
//-------------------------------------------------------------------------------------------------

void onVideoCmd(WinSocket *pSocket, WinScreenCaptureHelper::Settings &settings, const WinScreenCaptureHelper *pScreenCapturerHelper)
{
	IWinScreenCapture *pScreenCapture = pScreenCapturerHelper->checkSettings(settings);
	if (pScreenCapture)
	{
		const uint32_t nBufferSize = ((settings.nWidth * settings.nHeight * BPS) / 8) + 100;
		uint8_t *pBuffer = (uint8_t*)::malloc(nBufferSize);
		if (pBuffer)
		{
			CImage img;
			if (img.Create(settings.nWidth, settings.nHeight, BPS))
			{
				const double dExpectedTimeBetweenFrames = 1.0 / ((double)settings.nFPS);
				double dAvgTimeBetweenFrames = dExpectedTimeBetweenFrames;
				std::chrono::high_resolution_clock::time_point tpBegin = std::chrono::high_resolution_clock::now();
				if (pScreenCapture->captureScreenRect(settings.nX0, settings.nY0, settings.nCX, settings.nCY, img))
				{
					sendHttpOK(pSocket, "multipart/x-mixed-replace; boundary=\"SCREENCAP_MJPEG\"", nullptr, 0);
					for ( ; ; )
					{
						WinMemStream stream(pBuffer, nBufferSize, false);
						img.Save(&stream, ImageFormatJPEG);
						if (!sendHttpMultiPart(pSocket, "SCREENCAP_MJPEG", "image/jpeg", stream.getData(), stream.getSize()) ||
							!pScreenCapture->captureScreenRect(settings.nX0, settings.nY0, settings.nCX, settings.nCY, img))
							break;

						// Compute elapsed time and perform a sleep to meet FPS requirements (if needed)
						const std::chrono::high_resolution_clock::time_point tpEnd = std::chrono::high_resolution_clock::now();
						const std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(tpEnd - tpBegin);
						dAvgTimeBetweenFrames += dExpectedTimeBetweenFrames * (timeSpan.count() - dAvgTimeBetweenFrames); // 1 second estimation window
						if (dExpectedTimeBetweenFrames > dAvgTimeBetweenFrames)
							std::this_thread::sleep_for(std::chrono::duration<double>(dExpectedTimeBetweenFrames - dAvgTimeBetweenFrames));
						tpBegin = std::chrono::high_resolution_clock::now();
					}
				}
				else sendHttpInternalError(pSocket);
			}
			else sendHttpInternalError(pSocket);
			::free(pBuffer);
		}
		else sendHttpInternalError(pSocket);
		delete pScreenCapture;
	}
	else sendHttpInternalError(pSocket);
}
//-------------------------------------------------------------------------------------------------

void onHttpConnection(WinSocket *pSocket, void *pParam)
{
	const WinScreenCaptureHelper *pScreenCaptureHelper = reinterpret_cast<const WinScreenCaptureHelper*>(pParam);
	WinScreenCaptureHelper::Settings settings;

	// Process the request
	switch (processHttpRequest(pSocket, settings))
	{
		case Mode::Unknown:     onUnkownCmd(pSocket, settings, pScreenCaptureHelper); break;
		case Mode::HealthCheck: onHealthCheckCmd(pSocket, settings, pScreenCaptureHelper); break;
		case Mode::ScreenShot:  onScreenShotCmd(pSocket, settings, pScreenCaptureHelper); break;
		case Mode::Video:       onVideoCmd(pSocket, settings, pScreenCaptureHelper); break;
	}
	
	// Free up resources
	pSocket->close();
	delete pSocket;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	LOG_INFO("ScreenCaptureServer v1.0.5b. By @aviloria\n");
	
	// Parameter validation
	const char *strInterface = nullptr;
	uint16_t    nPort = 8080;
	uint16_t    nMaxConnections = 10;
	bool        bHide = false;
	bool        bMinimize = false;
	for (int n = 1; n < argc; ++n)
	{
		if (::strstr(argv[n], "-i:") == argv[n])
		{
			strInterface = argv[n] + 3;
		}
		else if (::strstr(argv[n], "-p:") == argv[n])
		{
			nPort = atoi(argv[n] + 3);
		}
		else if (::strstr(argv[n], "-c:") == argv[n])
		{
			nMaxConnections = atoi(argv[n] + 3);
		}
		else if (!::strcmp(argv[n], "-hide"))
		{
			bHide = true;
		}
		else if (!::strcmp(argv[n], "-minimize"))
		{
			bMinimize = true;
		}
		else
		{
			LOG_INFO("Syntax:\n");
			LOG_INFO("\t%s [options]\n", argv[0]);
			LOG_INFO("where 'options' is a combination of:\n");
			LOG_INFO("-i:<strInterface>      Server interface for accepting connections\n");
			LOG_INFO("                       Default value: ANY\n");
			LOG_INFO("-p:<nPort>             Server port number for accepting connections\n");
			LOG_INFO("                       Default value: 8080\n");
			LOG_INFO("-c:<nMaxConnections>   Maximum simultanous connections\n");
			LOG_INFO("                       Default value: 10\n");
			LOG_INFO("-hide                  Hide console on start\n");
			LOG_INFO("-minimize              Minimize console on start\n");
			return 1;
		}
	}

	// Check hidden/minimize flags
	if (bMinimize)
		ShowWindow(GetConsoleWindow(), SW_SHOWMINIMIZED);
	if (bHide)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	// Initialize the capturer helper
	WinScreenCaptureHelper helper;

	// Log server info
	LOG_INFO("\nStarting server...\n");
	LOG_INFO("+ Interface : %s\n", strInterface ? strInterface : "ANY");
	LOG_INFO("+ Port      : %u\n", nPort);
	LOG_INFO("+ Sim. conn.: %u\n", nMaxConnections);
	LOG_INFO("+ Caps.     : %s\n", helper.getCapabilitiesString().c_str());
	LOG_INFO("+ Disp.Devs.: %s\n", helper.getDisplayDevicesString().c_str());

	// Start server
	WinSocket server;
	server.startServer(strInterface, nPort, onHttpConnection, &helper, nMaxConnections);
	return 0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
