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
#ifndef __SOLDIN_SOCKET_H__
#define __SOLDIN_SOCKET_H__
#include <winsock2.h>
#include <buffer.h>
#include <crypt.h>
#include <shared.h>

class Socket {
public:
	Socket();
	Socket( SOCKET socket, sockaddr_in *addr, size_t len );
	~Socket();
	
	int         Connect( const char *hostname, uint16_t port );
	int         Listen( uint16_t port );
	Socket*     Accept();
	int         Receive( Buffer *dest );
	int         Send( char *src, size_t len );
	void        Disconnect();
	void        EnableEncryption( uint32_t key );
	void        DisableEncryption();
	const char *Address();
	inline bool Connected() const { return mConnected; }
	inline bool Encrypted() const { return ( mCrypt != NULL ); }

	/* Transmits the contents of a buffer. */
	int Send( Buffer *src )
	{
		if ( src != NULL && src->Size() > 0 )
			return Send( (char *)src->Content(), src->Size() );

		return 0;
	}

private:
	static int Initialize();
	static int Deinitialize();

	/* Checks if there is data available for retrieval. */
	bool DataAvailable()
	{
		fd_set set;
		struct timeval tm;
		tm.tv_sec = tm.tv_usec = 0;
		FD_ZERO( &set );
		FD_SET( mSocket, &set );

		select( mSocket + 1, &set, 0, 0, &tm );
		return FD_ISSET( mSocket, &set ) != 0;
	}

	/* Checks if data can be transmitted by this socket. */
	bool CanSend()
	{
		fd_set set;
		struct timeval tm;
		tm.tv_sec = tm.tv_usec = 0;
		FD_ZERO( &set );
		FD_SET( mSocket, &set );

		select( mSocket + 1, 0, &set, 0, &tm );
		return FD_ISSET( mSocket, &set ) != 0;
	}

	SOCKET         mSocket;
	uint16_t       mPort;
	bool           mConnected;
	static bool    mInitialized;
	static size_t  mSocketCount;
	static WSADATA mWsaData;
    SOCKADDR_IN    mAddr;
	char          *mIpAddr;
	Crypto        *mCrypt;
};

#endif /* __SOLDIN_SOCKET_H__ */
