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
#ifndef __SOLDIN_SQUARESESSION_H__
#define __SOLDIN_SQUARESESSION_H__

#include <socket.h>
#include <sessionmanager.h>
#include <messages.h>

#define MAX_IDLE_TIME 10
#define ERR_SESSION_NOTFOUND 1

/* Represents a squareserver that has connected to the gateway.  */
class SquareSession: public Session {
public:
	SquareSession( Socket *socket );
	~SquareSession();

	void            Update();
	void            Process( Buffer &buffer );
    void            Send( Buffer &buffer, uint16_t cmd, uint16_t type = 0x55E0 );
	inline Socket  *GetSocket() const  { return mSocket; }
	inline uint32_t GetSquareID() const { return mSquareId; }
	inline bool     IsConnected() const { return mSocket->Connected(); }

private:
	/* Message handlers. */
	void Msg_Auth          ( Buffer &packet );
	void Msg_Update        ( Buffer &packet );
	void Msg_GetSessionInfo( Buffer &packet );

	Socket  *mSocket;
	Buffer   mBufferIn;
	Buffer   mBufferOut;
	time_t   mLastUpdate;
	uint32_t mSquareId;
};

#endif /* __SOLDIN_SQUARESESSION_H__ */
