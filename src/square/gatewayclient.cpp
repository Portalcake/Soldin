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
#include <gatewayclient.h>
#include <time.h>
#include <log.h>
#include <sessionmanager.h>
#include <playersession.h>

extern uint16_t    cfg_square_port;
extern uint32_t    cfg_square_host;
extern uint32_t    cfg_square_capacity;
extern const char *cfg_square_name;

/* Creates a connection with a gateway server. */
GatewayClient::GatewayClient( const char *host, uint16_t port ): 
	mHostname( host ), 
	mPort( port )
{
	ServerLog.Write( "Connecting to gateway %s:%d.\n", E_INFO, mHostname, mPort );
	Connect();
}

/* Closes the connection with the gateway server. */
GatewayClient::~GatewayClient()
{
	ServerLog.Write( "Closing connection with gateway.\n", E_INFO );
}

/* Gets data send by the gateway and sends a update every couple of seconds. */
void GatewayClient::Update()
{
	if ( mSocket.Connected() )
	{
		/* Receive incoming data from the gateway. */
		int recv = mSocket.Receive( &mBufferIn );
		if ( recv == SOCKET_ERROR )
		{
			ErrorLog.Write( "Lost connection with gateway.\n", E_ERROR );
			return;
		}

		while ( mBufferIn.Size() >= 2 )
		{
			uint16_t size = mBufferIn[0] | ( mBufferIn[1] << 8 );
			if ( size <= mBufferIn.Size() )
			{
				char *data = (char *)malloc( size );
				mBufferIn.Slice( data, size, 0 );

				Process( Buffer( data, size ) );
				free( data );
			}
			else break;
		}

		/* Send a update to the gateway every <x> seconds. */
		time_t current = time(NULL);
		if ( mNextUpdate == 0 || current >= mNextUpdate )
		{
			Buffer updatepkt;

			updatepkt.WriteUInt32( 0 );
			Send( updatepkt, MSG_SQUARE_UPDATE );
			
			mNextUpdate = current + UPDATE_INTERVAL;
		}
	}
	else Connect();
}

/* Processes message received from the gateway. */
void GatewayClient::Process( Buffer &packet )
{
	packet.Seek( 6 );

	uint16_t cmd = *((uint16_t *)(packet.Content() + 4));
	switch ( cmd )
	{
		case MSG_SQUARE_SESSIONINFO: Msg_SessionInfo( packet ); break;
	}
}

/* Request session details from the gateway. */
void GatewayClient::GetSessionInfo( uint32_t session_id, const char *key )
{
	Buffer requestpkt;
	requestpkt.WriteUInt32( session_id );
	requestpkt.WriteString( key );

	Send( requestpkt, MSG_SQUARE_SESSIONINFO );
}

/* Session information received from the gateway. */
void GatewayClient::Msg_SessionInfo( Buffer &packet )
{
	uint32_t result     = packet.ReadUInt32();
	uint32_t session_id = packet.ReadInt32();

	PlayerSession *cl = SessionManager::At<PlayerSession>( session_id );
	if ( cl != NULL )
	{
		if (result != 0)
		{
			cl->mEOF = true;
			return;
		}
		else 
		{
			uint32_t char_id    = packet.ReadUInt32();
			uint32_t account_id = packet.ReadUInt32();

			cl->LoadCharacter( char_id, account_id );
		}
	}
}

/* Sends a packet to the gateway. */
void GatewayClient::Send( Buffer &data, uint16_t cmd, uint16_t type )
{
	/* Create the packet. */
	Buffer buffer_packet;
	buffer_packet.WriteUInt16( data.Size() + 6 );
	buffer_packet.WriteUInt16( type );
	buffer_packet.WriteUInt16( cmd );

	if (data.Size() > 0)
		buffer_packet.Write( data.Content(), data.Size() );

	mSocket.Send( (char *)buffer_packet.Content(), buffer_packet.Size() );
}

/* Connects to the gateway server. */
void GatewayClient::Connect()
{
	if ( !mSocket.Connected() )
	{
		int result;
		if ( ( result = mSocket.Connect( mHostname, mPort ) ) == 0 )
		{
			ServerLog.Write( "Connection with gateway established.\n", E_SUCCESS );

			Buffer infopkt;
			infopkt.WriteUInt32( cfg_square_host );
			infopkt.WriteUInt16( cfg_square_port );
			infopkt.WriteUInt32( cfg_square_capacity );
			infopkt.WriteString( cfg_square_name );

			Send( infopkt, MSG_SQUARE_AUTH );
		}
	}
}
