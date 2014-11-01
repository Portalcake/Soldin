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
#include <playersession.h>
#include <crypt.h>
#include <time.h>
#include <log.h>
#include <database.h>
#include <squaremanager.h>

/* Initializes a new instance of the CPlayerSession class. */
PlayerSession::PlayerSession( Socket *socket ):
	mSocket( socket ), 
	mAuthenticated( false ), 
	mCharacter( NULL ), 
	mAccount( NULL ),
	mStatus( ST_INLOBBY )
{
	mSessionId = INVALID_SESSION;
	mEOF       = false;
}

/* Releases all resources used by the session. */
PlayerSession::~PlayerSession()
{
	if ( mSocket != NULL )
	{
		delete mSocket;
	}
	if ( mAccount != NULL ) free( mAccount );
}

/* Updates the client by retrieving all the data send by the client and processing it. */
void PlayerSession::Update()
{
	int received = mSocket->Receive( &mBufferIn );
	if ( received == SOCKET_ERROR )
		return;

	while ( mBufferIn.Size() >= 2 )
	{
		uint16_t size = mBufferIn[0] | (mBufferIn[1] << 8);
		if ( size <= mBufferIn.Size() )
		{
			char *data = (char *)malloc( size );
			mBufferIn.Slice( data, size,  0);

			Process( Buffer( data, size ) );
			free( data );
		}
		else break;
	}

	if ( mBufferOut.Size() > 0 )
	{
		mSocket->Send( mBufferOut.Content(), mBufferOut.Size() );
		mBufferOut.Clear();
	}
}

/* Processes the specified packet. */
void PlayerSession::Process(Buffer& packet)
{
	struct HEADER {
		uint16_t size;
		uint16_t type;
		uint16_t command;
	} *header = (HEADER *)packet.Content();
	packet.Seek(6);

	switch ( header->command )
	{
		case MSG_CLIENTHASH:	     Msg_ClientHash( packet );        break;
		case MSG_LOGIN:		         Msg_Login( packet );             break;
		case MSG_DISCONNECT:	     Msg_Logout( packet );            break;
		case MSG_CHARACTER_CREATE:   Msg_CharacterCreate( packet );   break;
		case MSG_CHARACTER_DELETE:   Msg_CharacterDelete( packet );   break;
		case MSG_CHARACTER_SELECT:   Msg_CharacterSelect( packet );   break;
		case MSG_CHARACTER_DESELECT: Msg_CharacterDeselect( packet ); break;
		case MSG_SQUARE_SELECT:      Msg_SquareSelect( packet );      break;
		case MSG_SQUARE_LIST:        Msg_SquareList( packet );        break;
		case MSG_PING:               Msg_Ping( packet );              break;

		default:
			#if defined( _DEBUG )
			Unsupported( packet );
			#endif
			break;
	}
}

/* Sends a packet to the client. */
void PlayerSession::Send( Buffer &buffer, uint16_t cmd, uint16_t type )
{
	mBufferOut.WriteUInt16( buffer.Size() + 6 );
	mBufferOut.WriteUInt16( type );
	mBufferOut.WriteUInt16( cmd );
	if ( buffer.Size() > 0 )
	{
		mBufferOut.Write( buffer.Content(), buffer.Size() );
	}
}

/* Logs packets that are not supported. */
void PlayerSession::Unsupported( Buffer &packet )
{
	static int num = 0;

	/* Get the packet header. */
	struct HEADER {
		uint16_t m_size;
		uint16_t m_type;
		uint16_t m_command;
	} *header = (HEADER *)packet.Content();

	/* Log the packet. */
	DebugLog.Write( "Received unknown packet (size=%d, type=0x%X, cmd=0x%X)\n", E_DEBUG, header->m_size, header->m_type, header->m_command );

	char file_name[128];
	sprintf( file_name, "gw_%d_0x%x.bin", num, header->m_command );

	FILE *fp = fopen( file_name, "wb" );
	if ( fp != NULL )
	{
		fwrite( packet.Content(), packet.Size(), 1, fp );
		fclose( fp );
	}

	DebugLog.Write( "Packet logged as %s.\n", E_DEBUG, file_name );
	num++;
}

/* Sends the characterlist to the client. */
void PlayerSession::SendCharacterList()
{
	if ( !IsAuthenticated() )
		return;

	/* Send the character limit and the available character licenses. */
	Buffer licensepkt;
	licensepkt.WriteUInt32( mAccount->mMaxChars );
	licensepkt.WriteUInt32( HASH_LIST_CHARLICENCES );
	licensepkt.WriteUInt32( mAccount->mLicenseCount );
	if ( mAccount->mLicenseCount > 0 )
		for ( uint32_t i = 0; i < mAccount->mLicenseCount; i++ )
			licensepkt.WriteUInt32( mAccount->mLicenses[i] );

	Send( licensepkt, MSG_CHARACTER_LICENSE );

	/* Send the character list. */
	Buffer listpkt;
	listpkt.WriteUInt32( HASH_LIST_CHARACTERS );
	listpkt.WriteUInt32( mAccount->mCharacters.size() );
	if ( mAccount->mCharacters.size() > 0 )
	{
		uint32_t index = 0;
		for ( CharacterList::iterator i = mAccount->mCharacters.begin(); i != mAccount->mCharacters.end(); ++i, index++ )
		{
			CharacterData *chara = *i;

			listpkt.WriteUInt32( HASH_OBJ_CHARACTER +  index );
			listpkt.WriteWideString( UTF16( chara->mName ) );

			listpkt.WriteUInt32( 0 );
			listpkt.WriteUInt32( chara->mClassId );
			listpkt.WriteUInt16( chara->mLevel );
			listpkt.WriteUInt32( chara->mExperience );
			listpkt.WriteUInt16( chara->mPvpLevel );
			listpkt.WriteUInt32( chara->mPvpExperience );
			listpkt.WriteUInt16( chara->mWarLevel );
			listpkt.WriteUInt32( chara->mWarExperience );
			listpkt.WriteUInt16( chara->mRebirthLevel );
			listpkt.WriteUInt16( chara->mRebirthCount );

			struct tm *timeinfo = localtime( &chara->mLastPlayed );
			listpkt.WriteUInt32( HASH_DATETIME_LASTPLAYED );
			listpkt.WriteUInt16( timeinfo->tm_year + 1900 );
			listpkt.WriteUInt16( timeinfo->tm_mon + 1 );
			listpkt.WriteUInt16( timeinfo->tm_wday );
			listpkt.WriteUInt16( timeinfo->tm_hour );
			listpkt.WriteUInt16( timeinfo->tm_min );
			listpkt.WriteUInt16( timeinfo->tm_sec );
			listpkt.WriteUInt16( 0 );

			listpkt.WriteUInt32( HASH_LIST_EQUIPMENT );
			listpkt.WriteUInt32( chara->mEquipmentCount );
			for ( uint8_t j = 0; j < chara->mEquipmentCount; j++ )
			{
				listpkt.WriteUInt32( HASH_OBJ_EQUIPMENT + j );
				listpkt.WriteUInt32( chara->mEquipment[j].mId );

				listpkt.WriteUInt32( 0x613E5DCA );
				listpkt.Write( (byte *)"\0\0\0\0\0\0\0\0\0\0\0", 11 );
			}

			listpkt.WriteUInt32( HASH_LIST_STAGELICENCES );
			listpkt.WriteUInt32( 0 );
		}
	}
	Send( listpkt, MSG_CHARACTER_LIST );
}

/* Sends the squarelist to the client. */
void PlayerSession::SendSquareList()
{
	if ( !IsAuthenticated() || mCharacter == NULL )
		return;

	Buffer listpkt;
	listpkt.WriteUInt32( 0 );
	listpkt.WriteUInt32( HASH_LIST_SQUARES );
	listpkt.WriteUInt32( SquareManager::GetSquareCount() );

	Square *squares = SquareManager::GetList();
	for ( uint32_t i = 0, j = 0; i < MAX_SQUARES; i++ )
	{
		if ( squares[i].IsActive() != NULL )
		{
			listpkt.WriteUInt32( HASH_OBJ_SQUARE + j );
			listpkt.WriteWideString( UTF16( squares[i].mName ) );
			listpkt.WriteUInt32( squares[i].mStatus );
			listpkt.WriteUInt32( squares[i].mType );
			listpkt.WriteUInt32( squares[i].mCapacity );

			j++;
		}
	}
	Send( listpkt, MSG_SQUARE_LIST );
}
