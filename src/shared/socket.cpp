/*
 * Soldin - Lunia Server Emulator 
 * Copyright (c) 2010 Seipheroth
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. 
 */
#include <socket.h>
#include <ws2tcpip.h>

#define CAN_RECEIVE (this->DataAvailable())
#define CAN_SEND	(this->CanSend())

size_t  Socket::mSocketCount = 0;
bool    Socket::mInitialized = false;
WSADATA Socket::mWsaData;

#include <stdio.h>

/* Initializes a new instance of the Socket class. */
Socket::Socket():
	mIpAddr   ( NULL ), 
	mPort     ( 0 ), 
	mConnected( false ), 
	mCrypt    ( NULL )
{
	if ( !mInitialized )
		Initialize();
	
	mSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( mSocket != INVALID_SOCKET )
	{
		mSocketCount++;
	}
}

/* Initializes a new instance of the Socket class. */
Socket::Socket( SOCKET socket, sockaddr_in *addr, size_t len ): 
	mSocket   ( socket ), 
	mIpAddr   ( NULL ),
	mPort     ( 0 ), 
	mConnected( true ), 
	mCrypt    ( NULL )
{
	mSocketCount++;

	uint64_t non_blocking = 1;
	ioctlsocket( mSocket, FIONBIO, &non_blocking );

	if ( addr != NULL && len > 0 )
		memcpy( &mAddr, addr, len );
}

/* Closes the socket and releases all allocated resources. */
Socket::~Socket()
{
	if ( mIpAddr != NULL ) free( mIpAddr );
	if ( mCrypt != NULL ) delete mCrypt;

	if ( closesocket( mSocket ) != SOCKET_ERROR ) mSocketCount--;
	if ( mSocketCount == 0 )
	{
		Deinitialize();
	}
}

/* Connects to the specified host on the specified port. */
int Socket::Connect( const char *hostname, uint16_t port )
{
	mAddr.sin_family      = AF_INET;
	mAddr.sin_port        = htons( mPort = port );
	mAddr.sin_addr.s_addr = inet_addr( hostname );

	if ( connect( mSocket, (SOCKADDR *)&mAddr, sizeof( mAddr ) ) == SOCKET_ERROR )
		return WSAGetLastError();

	uint64_t non_blocking = 1;
	ioctlsocket( mSocket, FIONBIO, &non_blocking );

	mConnected = true;
	return 0;
}

/* Start listening on the specified port for connection requests. */
int Socket::Listen( uint16_t port )
{
	mAddr.sin_family      = AF_INET;
	mAddr.sin_port        = htons( mPort = port );
	mAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	if ( bind( mSocket, (SOCKADDR *)&mAddr, sizeof( mAddr ) ) == SOCKET_ERROR )
		return WSAGetLastError();

	uint64_t non_blocking = 1;
	ioctlsocket( mSocket, FIONBIO, &non_blocking );

	listen( mSocket, SOMAXCONN );
	return 0;
}

/* Accepts a incoming connection request. */
Socket* Socket::Accept()
{
	sockaddr_in addr;
	int len_addr = sizeof(sockaddr_in);

	SOCKET socket = accept( mSocket, (sockaddr *)&addr, &len_addr );
	if ( socket != INVALID_SOCKET )
		return new Socket( socket, &addr, len_addr );
	
	return NULL;
}

/* Initializes the socket subsystem. */
int Socket::Initialize()
{
	if ( mInitialized )
		return 0;

	int result = WSAStartup( MAKEWORD( 2, 2 ), &mWsaData );
	if ( result == 0 )
		mInitialized = true;

	return result;	
}

/* Deinitializes the socket subsystem. */
int Socket::Deinitialize()
{
	if (!mInitialized)
		return 0;

	int result = WSACleanup();
	if (result == 0)
		mInitialized = false;

	return result;
}

/* Receives data from the socket. */
int Socket::Receive( Buffer *dest )
{
	char   buffer[0x2000];
	size_t buffer_size = 0x2000;
	size_t size        = 0;
	size_t received    = 0;
	int    error       = 0;

	/* Loop until all data has been received. */
	while ( CAN_RECEIVE )
	{
		size = recv( mSocket, buffer, buffer_size, 0 );
		if ( size == SOCKET_ERROR )
		{
			error = WSAGetLastError();
			switch ( error )
			{
				case WSAENETDOWN:
				case WSAENETRESET:
				case WSAENOTCONN:
				case WSAEHOSTUNREACH:
				case WSAECONNABORTED:
				case WSAECONNRESET:
				case WSAETIMEDOUT:
				case WSAESHUTDOWN:
					Disconnect();
					return SOCKET_ERROR;

				default:
					break;
			}
		}
		else 
		{
			/* Check if the connection has been closed. */
			if (size == 0)
			{
				Disconnect();
				return SOCKET_ERROR;
			}

			/* Decrypt the received data. */
			if ( mCrypt != NULL ) mCrypt->Decrypt( (byte *)buffer, size );

			dest->Write( (byte *)buffer, size );
			received += size;
		}
	}
	return received;
}

/* Transmits the data in the buffer pointed to by src. */
int Socket::Send( char *src, size_t len )
{
	size_t sent       = 0;
	size_t sent_total = 0;
	size_t bytes_left = len;
	int    error      = 0;

	/* Loop until all data has been sent. */
	while ( CAN_SEND )
	{
		sent = send( mSocket, src + sent_total, bytes_left, 0 );
		if ( sent == SOCKET_ERROR )
		{
			error = WSAGetLastError();
			switch ( error )
			{
				case WSAENETDOWN:
				case WSAENETRESET:
				case WSAENOTCONN:
				case WSAEHOSTUNREACH:
				case WSAECONNABORTED:
				case WSAECONNRESET:
				case WSAETIMEDOUT:
				case WSAESHUTDOWN:
					Disconnect();
					return SOCKET_ERROR;

				case WSAEWOULDBLOCK:
				case WSAEINPROGRESS:
					Disconnect();
					return sent_total;

				default:
					break;
			}
		}

		sent_total += sent;
		if ( sent_total == len )
			break;

		bytes_left = len - sent_total;
	}
	return sent_total;
}

/* Closes the connection and socket. */
void Socket::Disconnect()
{
	if ( !mConnected )
		return;

	/* Close the current socket and get a new one. */
	closesocket( mSocket );
	mSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	mConnected = false;
}

/* Enables encryption on the socket. */
void Socket::EnableEncryption( uint32_t key )
{
	if ( mCrypt == NULL )
	{
		mCrypt = new Crypto();
	}
	mCrypt->SetKey( key );
}

/* Disables encryption on the socket. */
void Socket::DisableEncryption()
{
	if ( mCrypt != NULL )
	{
		delete mCrypt;
		mCrypt = NULL;
	}
}

/* Gets the IP addres of the remote endpoint. */
const char *Socket::Address()
{
	if ( mIpAddr == NULL )
	{
		mIpAddr = (char *)malloc(NI_MAXHOST);
		memset( mIpAddr, 0, NI_MAXHOST );

		int result = getnameinfo( (struct sockaddr *)&mAddr, sizeof( struct sockaddr ), mIpAddr, NI_MAXHOST, 0, 0, NI_NUMERICHOST );
		if ( result != 0 )
		{
			free( mIpAddr );
			mIpAddr = NULL;

			return NULL;
		}
	}

	return mIpAddr;
}
