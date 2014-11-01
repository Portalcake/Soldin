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
#ifndef __SOLDIN_GATEWAYCLIENT_H__
#define __SOLDIN_GATEWAYCLIENT_H__

#include <socket.h>
#include <buffer.h>
#include <vector>

#define UPDATE_INTERVAL 5

#define MSG_SQUARE_AUTH        0x0001
#define MSG_SQUARE_UPDATE      0x0002
#define MSG_SQUARE_SESSIONINFO 0x0003

class GatewayClient {
public:
	GatewayClient( const char *host, uint16_t port );
	~GatewayClient();

	void        Update();
	void        Send( Buffer &data, uint16_t cmd, uint16_t type = 0x55E0 );
	void        Process( Buffer &packet );
	void        GetSessionInfo( uint32_t session_id, const char *key );

	const char *GetHostname() { return mHostname; }
	uint16_t	GetPort()     { return mPort; }
	Socket     *GetSocket()   { return &mSocket; }

private:
	void Connect();

	/* Packet handlers. */
	void Msg_SessionInfo( Buffer &packet );

	const char *mHostname;
	uint16_t    mPort;
	Socket      mSocket;
	time_t      mNextUpdate;
	Buffer      mBufferIn;
};

#endif /* __SOLDIN_GATEWAYCLIENT_H__ */
