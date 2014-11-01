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
#include <time.h>
#include <log.h>

/* Configuration Globals */
extern const char *cfg_gateway_ip;

/* Generates a encryption key for the client. */
void PlayerSession::Msg_ClientHash( Buffer &buffer )
{
	uint32_t crypt_key = Crypto::GenerateKey();

	time_t raw;
	struct tm *timeinfo;
	time( &raw );
	timeinfo = localtime( &raw );
	
	Buffer cryptpkt;
	cryptpkt.WriteUInt32( 0 );
	cryptpkt.WriteUInt32( HASH_DATETIME_CONNECTED );
	cryptpkt.WriteUInt16( timeinfo->tm_year );
	cryptpkt.WriteUInt16( timeinfo->tm_mon );
	cryptpkt.WriteUInt16( timeinfo->tm_wday );
	cryptpkt.WriteUInt16( timeinfo->tm_hour );
	cryptpkt.WriteUInt16( timeinfo->tm_min );
	cryptpkt.WriteUInt16( timeinfo->tm_sec );
	cryptpkt.WriteUInt16( 0 );
	cryptpkt.WriteString( cfg_gateway_ip );
	cryptpkt.WriteUInt32( crypt_key );
	Send( cryptpkt, MSG_CLIENTHASH );

	mSocket->EnableEncryption( crypt_key );
}

/* Handles a login attemp from the client. */
void PlayerSession::Msg_Login( Buffer &packet )
{
	const wchar_t *username = packet.ReadWideString();
	const char    *password = packet.ReadString();

	Buffer resultpkt;

	/* Load the account. */
	mAccount = DB::Account_Load( UTF8( username ), NULL );
	if ( mAccount == NULL )
	{
		resultpkt.WriteUInt32( ERR_LOGIN_NOTFOUND );
		resultpkt.WriteWideString( username );
		resultpkt.WriteUInt32( 0 );

		Send( resultpkt, MSG_LOGIN );
		return;
	}
	
	/* Verify the passwords. */
	if ( _stricmp( mAccount->mPassword, password ) != 0 )
	{
		resultpkt.WriteUInt32( ERR_LOGIN_INVALIDPASSWD );
		resultpkt.WriteWideString( username );
		resultpkt.WriteUInt32( 0 );
	}
	else
	{
		uint32_t result = ERR_NONE;
		
		/* Check the account status. */
		PlayerSession *cl = SessionManager::FindByName<PlayerSession>( mAccount->mName );
		if ( cl != NULL ) /* Already logged in? */
		{
			if ( cl->GetStatus() == ST_INLOBBY )
			{
				result = ERR_LOGIN_ALREADYCONLOBBY;
			}
			else result = ERR_LOGIN_ALREADYCONSERVER;
		}
		else
		{
			switch ( mAccount->mStatus )
			{
				case 0: mAuthenticated = true; break;
				case 1: result = ERR_LOGIN_ACCBLOCKED; break;
				case 2: result = ERR_LOGIN_ACCDELETED; break;
			}
		}

		resultpkt.WriteUInt32( result );
		resultpkt.WriteWideString( username );
		resultpkt.WriteUInt32( 0 );
	}
	Send( resultpkt, MSG_LOGIN );

	if ( mAuthenticated ) 
	{
		SendCharacterList();

		mName = mAccount->mName;
	}
}

/* Closes the connection with the client. */
void PlayerSession::Msg_Logout( Buffer &packet )
{
	mAuthenticated = false;
	mEOF           = true;
}

/* Processes the request to create a new character. */
void PlayerSession::Msg_CharacterCreate( Buffer &packet )
{
	Buffer result;

	static byte blank_character[] = {
		0xDA, 0x3D, 0x71, 0xBC, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x95, 0x24, 0x34, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0x1A,
		0x69, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x66, 0xD1, 0x3C, 0x39, 0x00, 0x00, 0x00, 0x00,
		0xF3, 0xE8, 0x3C, 0x39, 0x00, 0x00, 0x00, 0x00 
	};

	const char *char_name  = UTF8( packet.ReadWideString() );
	uint32_t    char_class = packet.ReadUInt32();

	/* Check if the requested name is available. */
	if ( DB::Character_Exists( char_name ) )
	{
		result.WriteUInt32( ERR_CHARCREATE_NAMETAKEN );
		result.Write( blank_character, 72 );
	}
	else
	{
		/* Try to create the character. */
		uint32_t char_id = DB::Character_Create( mAccount->mId, char_class, char_name );
		if ( char_id == 0 )
		{
			result.WriteUInt32( ERR_CHARCREATE_FAILED );
			result.Write( blank_character, 72 );
		}
		else
		{
			CharacterData *chara = DB::Character_Load( char_id, NULL );
			if ( chara == NULL )
			{
				result.WriteUInt32( ERR_CHARCREATE_FAILED );
				result.Write( blank_character, 72 );
			}
			else
			{
				uint32_t i = mAccount->mCharacters.size();

				result.WriteUInt32( ERR_NONE );
				result.WriteUInt32( HASH_OBJ_NEWCHARACTER );
				result.WriteWideString( UTF16( chara->mName) );

				result.WriteUInt32( 0 );
				result.WriteUInt32( chara->mClassId );
				result.WriteUInt16( chara->mLevel );
				result.WriteUInt32( chara->mExperience );
				result.WriteUInt16( chara->mPvpLevel);
				result.WriteUInt32( chara->mPvpExperience );
				result.WriteUInt16( chara->mWarLevel );
				result.WriteUInt32( chara->mWarExperience );
				result.WriteUInt16( chara->mRebirthLevel );
				result.WriteUInt16( chara->mRebirthCount );

				struct tm *timeinfo = localtime( &chara->mLastPlayed );
				result.WriteUInt32( HASH_DATETIME_LASTPLAYED );
				result.WriteUInt16( timeinfo->tm_year );
				result.WriteUInt16( timeinfo->tm_mon );
				result.WriteUInt16( timeinfo->tm_wday );
				result.WriteUInt16( timeinfo->tm_hour );
				result.WriteUInt16( timeinfo->tm_min );
				result.WriteUInt16( timeinfo->tm_sec );
				result.WriteUInt16( 0 );

				result.WriteUInt32( HASH_LIST_EQUIPMENT );
				result.WriteUInt32( chara->mEquipmentCount );
				for ( uint8_t j = 0; j < chara->mEquipmentCount; j++ )
				{
					result.WriteUInt32( HASH_OBJ_EQUIPMENT + j );
					result.WriteUInt32( chara->mEquipment[i].mId );
					result.WriteUInt32( 0x613E5DCA );
					result.Write( (byte *)"\0\0\0\0\0\0\0\0\0\0\0", 11 );
				}

				result.WriteUInt32( HASH_LIST_STAGELICENCES );
				result.WriteUInt32( 0 );
			}
		}
	}
	Send( result, MSG_CHARACTER_CREATE );
}

/* Processes the request to delete a character. */
void PlayerSession::Msg_CharacterDelete( Buffer &packet )
{
	if ( !IsAuthenticated() || mAccount->mCharacters.size() == 0 )
		return;

	Buffer resultpkt;

	const wchar_t *widestr  = packet.ReadWideString();
	const char    *charname = UTF8( widestr );


	for ( CharacterList::iterator i = mAccount->mCharacters.begin(); i != mAccount->mCharacters.end(); ++i )
	{
		if ( _stricmp( charname, ( *i )->mName ) == 0 )
		{
			DB::Character_Delete( ( *i )->mId );

			resultpkt.WriteUInt32( ERR_NONE );
			resultpkt.WriteWideString( widestr );
			Send( resultpkt, MSG_CHARACTER_DELETE );

			mAccount->mCharacters.erase( i );
			return;
		}
	}

	resultpkt.WriteUInt32( ERR_DELCHAR_NOTFOUND );
	resultpkt.WriteWideString( L"" );
	Send( resultpkt, MSG_CHARACTER_DELETE );
}

/* Selects a character from the character list. */
void PlayerSession::Msg_CharacterSelect( Buffer &packet )
{
	if ( !IsAuthenticated() || mAccount->mCharacters.size() == 0 )
		return;

	const wchar_t *widestr  = packet.ReadWideString();
	const char    *charname = UTF8( widestr );

	Buffer resultpkt;
	for ( CharacterList::iterator i = mAccount->mCharacters.begin(); i != mAccount->mCharacters.end(); ++i )
	{
		if ( _stricmp( charname, ( *i )->mName ) == 0 )
		{
			mCharacter = *i;
			
			resultpkt.WriteUInt32( ERR_NONE );
			resultpkt.WriteWideString( widestr );
			resultpkt.WriteUInt32( 0 );
			Send( resultpkt, MSG_CHARACTER_SELECT );
			return;
		}
	}

	resultpkt.WriteUInt32( ERR_SELCHAR_CANCEL );
	resultpkt.WriteWideString( L"" );
	resultpkt.WriteUInt32( 0 );
	Send( resultpkt, MSG_CHARACTER_SELECT );
}

/* Deselects the currently selected character and tells the client to return to the character selection screen. */
void PlayerSession::Msg_CharacterDeselect( Buffer &packet )
{
	if ( !IsAuthenticated() || mAccount->mCharacters.size() == 0 )
		return;

	mCharacter = NULL;

	Buffer resultpkt;
	resultpkt.WriteUInt32( 0 );
	Send( resultpkt, MSG_CHARACTER_DESELECT );
}

/* Selects a square and sends the details to the client. */
void PlayerSession::Msg_SquareSelect( Buffer &packet )
{
	Buffer resultpkt;

	mSquare = SquareManager::Find( UTF8( packet.ReadWideString() ) );
	if ( mSquare != NULL )
	{
		if ( mSquare->mStatus == STATUS_FULL )
		{
			resultpkt.WriteUInt32( ERR_SELSQUARE_FULL );
			resultpkt.WriteString( "" );
			resultpkt.WriteUInt16( 0 );
			resultpkt.WriteString( "" );
		}
		else
		{
			resultpkt.WriteUInt32( ERR_NONE );
			resultpkt.WriteString( inet_ntoa( mSquare->mHostAddr ) );
			resultpkt.WriteUInt16( mSquare->mPort );
			resultpkt.WriteString( mSessionKey );
		}
	}
	else
	{
		resultpkt.WriteUInt32( ERR_SELSQUARE_NOTFOUND );
		resultpkt.WriteString( "" );
		resultpkt.WriteUInt16( 0 );
		resultpkt.WriteString( "" );
	}
	Send( resultpkt, MSG_SQUARE_DETAILS );
}

/* Sends the square list to the client. */
void PlayerSession::Msg_SquareList( Buffer &packet ) { SendSquareList(); }

/* Responds to a ping message. */
void PlayerSession::Msg_Ping( Buffer &packet ) { }
