/*************************************************************************************************/
/* Project: Screen capture server                                                                */
/* Author: @aviloria (GitHub)                                                                    */
/*-----------------------------------------------------------------------------------------------*/
/* Copyleft license                                                                              */
/*    YOU ARE ALLOWED TO FREELY DISTRIBUTE COPIES AND MODIFIED VERSIONS OF THIS WORK WITH        */
/*    THE STIPULATION THAT THE SAME RIGHTS BE PRESERVED IN DERIVATIVE WORKS CREATED LATER.       */
/*************************************************************************************************/
//-------------------------------------------------------------------------------------------------
#ifndef __WINSOCKET_H__
#define __WINSOCKET_H__
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
#include <stdint.h>
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
* Class to implement common Windows Socket operations
*/
class WinSocket
{
public:
	typedef void(*FnConnection) (WinSocket *pSocket, void *pParam);

public:
	/**
	 * Default constructor
	 */
	WinSocket();

	/**
	 * Destructor
	 */
	~WinSocket();

	/**
	 * Starts a server.
	 * It runs an infinite loop accepting socket connections.
	 * For each connection, in runs @p fnOnConnection method in a separate thread.
	 *
	 * @param strInterface A string containing the interface address to listen up. It can be null to specify 'any interface' in the system.
	 * @param nPort The port number for listening
	 * @param fnOnConnection A function to run (in a separate thread) to process each connection
	 * @param pParam An optional parameter to be passed to @p fnOnConnection. It can be null
	 * @param nMaxConnections Maximun simultaneous connections allowed.
	 * @return true on succeed, false otherwise. Take into account that if succeeded, it enters in a infinite loop.
	 */
	bool startServer(const char *strInterface /*=nullptr*/, uint16_t nPort, FnConnection fnOnConnection, void *pParam = nullptr, uint32_t nMaxConnections = SOMAXCONN);

	/**
	 * Opens a connection in client mode.
	 *
	 * @param strAddress A string containing the address to connect
	 * @param nPort The port number used in the connexion
	 * @return true on succeed, false otherwise
	 */
	bool open(const char *strAddress, uint16_t nPort);

	/**
	 * Closes a connection in client mode.
	 *
	 * @return true on succeed, false otherwise
	 */
	bool close();

	/**
	 * Sets the timeout value used for reading and writting
	 *
	 * @param nTimeOutInMillisecs Timeout value (in milliseconds)
	 */
	void setTimeOut(uint32_t nTimeOutInMillisecs);

	/**
	* Reads data into a buffer
	* This function will try to receive all available data for reading until the buffer is full.
	* Uses the timeout value set with setTimeOut()
	*
	* @param pBuffer Pointer to a buffer receiving the data
	* @param nBufferSize Buffer size
	* @return Number of readed bytes. It can be zero in timeout, broken connection and error cases.
	*/
	uint32_t read(uint8_t *pBuffer, uint32_t nBufferSize);

	/**
	* Writes data from a buffer to the socket connection
	* Uses the timeout value set with setTimeOut()
	*
	* @param pBuffer Pointer to a buffer containing the data to be sent
	* @param nBufferSize Buffer size
	* @return Number of sent bytes. It can be less than @p nBufferSize in timeout, broken connection and error cases.
	*/
	uint32_t write(const uint8_t *pBuffer, uint32_t nBufferSize);

private:
	SOCKET       _sSocket;
	sockaddr_in  _addrInfo;
	timeval      _tvTimeout;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
#endif // __WINSOCKET_H__
//-------------------------------------------------------------------------------------------------
