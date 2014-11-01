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
#include <squaresession.h>
#include <squaremanager.h>
#include <sessionmanager.h>
#include <playersession.h>
#include <time.h>
#include <log.h>

/* Initializes a new instance of the Client class. */
SquareSession::SquareSession( Socket *socket ): 
	mSocket    ( socket ), 
	mSquareId  ( INVALID_SQUARE ), 
	mLastUpdate( time( NULL ) )  
{ 
	mSessionId = INVALID_SESSION;
	mEOF = false;
}

/* Releases all resources used by the session. */
SquareSession::~SquareSession()
{
	if ( mSquareId != INVALID_SQUARE )
	{
		SquareManager::Remove( mSquareId );
	}

	if ( mSocket ) delete mSocket;
}

/* Updates the client by retrieving all the data send by the client and processing it. */
void SquareSession::Update()
{
	int received = mSocket->Receive( &mBufferIn );
	if (received == SOCKET_ERROR)
		return;

	/* If we receive no data for a specific period of time close the connection. */
	time_t current = time(NULL);
	if ( received == 0 && ( current - mLastUpdate ) >= MAX_IDLE_TIME )
	{
		mSocket->Disconnect();
		mEOF = true;
		
		return;
	}
	else if (received > 0) mLastUpdate = current;

	/* Process all packets in the incoming data buffer. */
	while ( mBufferIn.Size() >= 2 )
	{
		size_t len = mBufferIn[0] | (mBufferIn[1] << 8);
		if ( len <= mBufferIn.Size() )
		{
			char *data = (char *)malloc( len );
			mBufferIn.Slice( data, len, 0 );

			Process( Buffer( data, len ) );
			free( data );
		}
		else break;
	}

	/* Send the content of the outgoing buffer to the square server. */
	if ( mBufferOut.Size() > 0 )
	{
		mSocket->Send( (char *)mBufferOut.Content(), mBufferOut.Size() );
		mBufferOut.Clear();
	}
}

/* Processes the specified packet. */
void SquareSession::Process( Buffer& packet )
{
	/* Get the packet header. */
	struct PacketHeader {
		uint16_t size;
		uint16_t type;
		uint16_t command;
	} *header = (PacketHeader *)packet.Content();
	packet.Seek( 6 );

	/* Process the command. */
	switch ( header->command )
	{
		case MSG_SQUARE_AUTH:        Msg_Auth          ( packet ); break;
		case MSG_SQUARE_UPDATE:      Msg_Update        ( packet ); break;
		case MSG_SQUARE_SESSIONINFO: Msg_GetSessionInfo( packet ); break;
	}
}

/* Sends a packet to the square server. */
void SquareSession::Send( Buffer &buffer, uint16_t cmd, uint16_t type )
{
	mBufferOut.WriteUInt16( ( buffer.Size() + 6 ) );
	mBufferOut.WriteUInt16( type );
	mBufferOut.WriteUInt16( cmd );

	if ( mBufferOut.Size() > 0 )
	{
		mBufferOut.Write( buffer.Content(), buffer.Size() );
	}
}

/* Adds the square to the manager. */
void SquareSession::Msg_Auth( Buffer &packet )
{
	uint32_t hostaddr = packet.ReadUInt32();
	uint16_t port     = packet.ReadUInt16();
	uint32_t capacity = packet.ReadUInt32();
	const char *name  = packet.ReadString();

	/* Register the square. */
	mSquareId = SquareManager::Add( name, hostaddr, port, capacity, this );
	if ( mSquareId == INVALID_SQUARE )
	{
		mSocket->Disconnect();
		mEOF = true;

		ServerLog.Write( "[%d][%d][SQUARE] Connection rejected, square limit reached.\n", E_WARNING, mSessionId, mSquareId );
	}
	else
	{
		Square *square = SquareManager::At( mSquareId );
		ServerLog.Write( "[%d][%d][SQUARE] Square '%s' (%s:%d) has been added.\n", E_NOTICE, 
			mSessionId, mSquareId, square->mName, inet_ntoa( square->mHostAddr ), square->mPort );
	}
};

/* Updates the number of players on the square. */
void SquareSession::Msg_Update( Buffer &packet )
{
	Square *square = SquareManager::At( mSquareId );
	if ( square != NULL )
	{
		square->mOnlineUsers = packet.ReadUInt32();
	}
}

/* Square has requested information about a session.  */
void SquareSession::Msg_GetSessionInfo( Buffer &packet )
{
	uint32_t       session_id  = packet.ReadUInt32();
	const char    *session_key = packet.ReadString();
	PlayerSession *client;

	/* Generate the response. */
	Buffer infopkt;
	if ( ( client = SessionManager::Find<PlayerSession>( session_key ) ) != NULL )
	{
		infopkt.WriteUInt32( 0 );
		infopkt.WriteUInt32( session_id );
		infopkt.WriteUInt32( client->GetCharacterID() );
		infopkt.WriteUInt32( client->GetAccountID() );
	}
	else 
	{
		infopkt.WriteUInt32( ERR_SESSION_NOTFOUND );
		infopkt.WriteUInt32( session_id );
	}
	Send( infopkt, MSG_SQUARE_SESSIONINFO );
}
