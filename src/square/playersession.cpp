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
#include <gatewayclient.h>
#include <log.h>
#include <stage.h>

extern GatewayClient *g_gateway;

/* Initializes a new instance of the PlayerSession class. */
PlayerSession::PlayerSession( Socket *socket ): 
	mSocket( socket ), 
	mProgress( 0.0f ), 
	mMoving( false ), 
	mCharacter( NULL )
{
	mSessionId = INVALID_SESSION;
	mEOF       = false;

	SetEncryptionKey( Crypto::GenerateKey() );

	mPosition.x  = 1200.0f;
	mPosition.y  = 0.0f;
	mPosition.z  = 610.0f;

	mDirection.x = 0;
	mDirection.y = 0;
	mDirection.z = -1.0f;
}


/* Releases all resources used by the session. */
PlayerSession::~PlayerSession()
{
	if ( mSocket != NULL )
	{
		delete mSocket;
	}

	if ( mCharacter ) free( mCharacter );
}

/* Sets the encryption key for this session. */
void PlayerSession::SetEncryptionKey( uint32_t key )
{
	mSocket->EnableEncryption( key );

	Buffer keypkt;
	keypkt.WriteUInt32( key );
	Send( keypkt, MSG_LOAD_ENCRYPTIONKEY );
}

/* Updates the client by retrieving all the data send by the client and processing it. */
void PlayerSession::Update()
{
	int received = mSocket->Receive( &mBufferIn );
	if (received == SOCKET_ERROR)
		return;

	/* Process incoming data. */
	while ( mBufferIn.Size() >= 2 )
	{
		uint16_t size = mBufferIn[0] | ( mBufferIn[1] << 8 );
		printf("%d\n", size);
		if ( size <= mBufferIn.Size() )
		{
			char *data = (char *)malloc( size );
			mBufferIn.Slice( data, size, 0 );

			Process( Buffer( data, size ) );
			free( data );
		}
		else break;
	}

	/* Send all outgoing data. */
	if ( mBufferOut.Size() > 0 )
	{
		mSocket->Send( mBufferOut.Content(), mBufferOut.Size() );
		mBufferOut.Clear();
	}
}

/* Authentication succesful, load character details. */
void PlayerSession::LoadCharacter( uint32_t char_id, uint32_t account_id )
{
	mAccount = DB::Account_Load( account_id, NULL, false );
	if ( mAccount == NULL )
	{
		mEOF = true;
		return;
	}

	mCharacter = DB::Character_Load( char_id, NULL );
	if ( mCharacter == NULL )
	{
		mEOF = true;
		return;
	}
	#if defined( _DEBUG )
	DebugLog.Write( "[%d] Loaded account %s (aid: %d)\n", E_INFO, mSessionId, mAccount->mName, mAccount->mId );
	DebugLog.Write( "[%d] Loaded character %s (cid: %d)\n", E_INFO, mSessionId, mCharacter->mName, mCharacter->mId );
	#endif
	
	SendCharacterInfo();
}

/* Processes the specified packet. */
void PlayerSession::Process( Buffer& packet )
{
	struct HEADER {
		uint16_t mSize;
		uint16_t mType;
		uint16_t mCommand;
	} *header = (HEADER *)packet.Content();
	packet.Seek(6);

	switch ( header->mCommand )
	{
		case MSG_LOAD_AUTHENTICATE:      Msg_Load_Authenticate( packet );      break;
		case MSG_LOAD_PROGRESS:	         Msg_Load_Progress( packet );          break;
		case MSG_LOAD_DONE:	             Msg_Load_Done (packet );              break;
		case MSG_CHARACTER_MOVE:         Msg_Character_Move( packet );         break;
		case MSG_INVENTORY_GETBAGITEMS:  Msg_Inventory_GetBagItems( packet );  break;
		case MSG_INVENTORY_GETBANKITEMS: Msg_Inventory_GetBankItems( packet ); break;
		case MSG_SKILLS_GETQUICKSLOTS:   Msg_Skills_GetQuickSlots( packet );   break;
		case MSG_CHAT_MESSAGE:           Msg_Chat_Message( packet );           break;

		case MSG_SHOP_ENTER:             Msg_Shop_Enter( packet );             break;
		case MSG_SHOP_LEAVE:             Msg_Shop_Leave( packet );             break;
		case MSG_SHOP_BUY:               Msg_Shop_Buy( packet );               break;
		case MSG_SHOP_SELL:              Msg_Shop_Sell( packet );              break;

		case MSG_BANK_DEPOSIT: Msg_Bank_Deposit( packet ); break;
		case MSG_BANK_WITHDRAW: Msg_Bank_Withdraw( packet ); break;
		//case MSG_BANK_MOVE: Msg_Bank_Move( packet ); break;

		case 0x482F:
			{
				Buffer result;
				result.Write( packet.Content() + 6, 12 );
				Send( result, 0x482F );
			}
			break;

		default:
			Unsupported( packet );
			break;
	}
}

/* Sets the action/animation of the character. */
void PlayerSession::SetAction( uint32_t action )
{
	Buffer actionpkt;
	actionpkt.WriteUInt32( mCharacter->mId );
	actionpkt.WriteUInt32( action );
	actionpkt.WriteFloat( mPosition.x );
	actionpkt.WriteFloat( mPosition.y );
	actionpkt.WriteFloat( mPosition.z );
	actionpkt.WriteFloat( mDirection.x );
	actionpkt.WriteFloat( mDirection.y );
	actionpkt.WriteFloat( mDirection.z );
	actionpkt.WriteFloat( 0 );

	mStage->Send( actionpkt, MSG_CHARACTER_SETACTION );
}

/* Sends a textbox message to the client. */ 
void PlayerSession::SendBoardMessage( const char *from, const char *message )
{
	Buffer chatpkt;
	chatpkt.WriteUInt32( 0xFFFFFFFF ); // Character ID.
	chatpkt.WriteUInt32( 8 );          // Type.
	
	char msg_data[1024];
	sprintf( msg_data, "1 %s %s", from, message );
	chatpkt.WriteWideString( UTF16( msg_data ) );

	Send( chatpkt, MSG_CHAT_MESSAGE );
}

/* Sends a packet to the client. */
void PlayerSession::Send( Buffer &buffer, uint16_t cmd, uint16_t type )
{
	if ( buffer.Size() == 0 )
		return;

	mBufferOut.WriteUInt16( buffer.Size() + 6 );
	mBufferOut.WriteUInt16( type );
	mBufferOut.WriteUInt16( cmd );

	mBufferOut.Write( buffer.Content(), buffer.Size() );
}

/* Logs packets that are not supported. */
void PlayerSession::Unsupported( Buffer &packet )
{
	static int packet_num = 1;

	/* Get the packet header. */
	struct HEADER {
		uint16_t mSize;
		uint16_t mType;
		uint16_t mCommand;
	} *header = (HEADER *)packet.Content();

	/* Log the packet. */
	DebugLog.Write( "Received unknown packet (size=%d, type=0x%X, cmd=0x%X)\n", E_DEBUG, header->mSize, header->mType, header->mCommand );

	char file_name[128];
	sprintf( file_name, "packet_%d_0x%x.bin", packet_num++, header->mCommand );

	FILE *f = fopen( file_name, "wb" );
	fwrite (packet.Content(), packet.Size(), 1, f );
	fclose( f );

	DebugLog.Write( "Packet logged as %s.\n", E_DEBUG, file_name );
}



void PlayerSession::SendCharacterInfo()
{
	Buffer unknownpkt;
	unknownpkt.WriteWideString( UTF16( mCharacter->mName ) ); // Charactername.
	unknownpkt.WriteUInt32( 0x529E424F ); // Stagegroup Hash.
	unknownpkt.WriteUInt32( 0x0330A106 ); // Stagegroup ID.
	unknownpkt.WriteUInt16( 0 ); // Level.
	unknownpkt.WriteUInt32( 0x393C107A ); // Unknown list hash.
	unknownpkt.WriteUInt32( 0 );
	Send( unknownpkt, 0xE9FD );

	/* Character information. */
	Buffer infopkt;
	infopkt.WriteUInt32( mCharacter->mClassId );
	infopkt.WriteUInt16( mCharacter->mLevel );
	infopkt.WriteUInt32( mCharacter->mExperience );
	infopkt.WriteUInt16( mCharacter->mPvpLevel );
	infopkt.WriteUInt32( mCharacter->mPvpExperience );
	infopkt.WriteUInt16( mCharacter->mWarLevel );
	infopkt.WriteUInt32( mCharacter->mWarExperience );
	infopkt.WriteUInt16( mCharacter->mRebirthLevel );
	infopkt.WriteUInt16( mCharacter->mRebirthCount );
	infopkt.WriteUInt32( mCharacter->mMoney );
	infopkt.Write( (byte *)"\0\0\0\0\0\0", 6 ); // Unknown.
	infopkt.WriteUInt16( 0 );                   // Skill points.
	infopkt.WriteUInt16( 0 );                   // Added skill points.
	infopkt.WriteByte( 0 );                     // Spectator?
	infopkt.WriteByte( 0 );                     // Unknown...
	infopkt.WriteByte( 0 );                     // Unknown...
	Send( infopkt, MSG_CHARACTER_INFO );

	uint32_t bag_id = 0;

	/* Send a list of all the available bags. */
	Buffer bagspkt;
	bagspkt.WriteUInt32( 0x393CE1CC );  // Bag list hash.

#define HASH_BAGLICENSE 0xDAA95362
#define HASH_DATE_BAGEXPIRATION 0x1769D5AF



	bagspkt.WriteUInt32( mCharacter->mBagLicenses->size() );

	printf( "bag_lic = %d\n", mCharacter->mBagLicenses->size() );

	for ( std::vector<BagLicense *>::iterator i = mCharacter->mBagLicenses->begin(); i != mCharacter->mBagLicenses->end(); ++i, bag_id++ )
	{
		BagLicense *l = *i;
		bagspkt.WriteUInt32( HASH_BAGLICENSE + bag_id );
		bagspkt.WriteUInt32( l->mIndex );

		bagspkt.WriteUInt32( HASH_DATE_BAGEXPIRATION );
		if ( l->mStatus == 2 ) /* Is this a permanent bag? */
		{
			bagspkt.WriteUInt32( 9999 );
			bagspkt.WriteUInt32( 1 );
			bagspkt.WriteUInt32( 1 );
			bagspkt.WriteUInt32( 0 );
			bagspkt.WriteUInt32( 0 );
			bagspkt.WriteUInt32( 0 );
			bagspkt.WriteUInt32( 0 );
			bagspkt.WriteByte( 0 );
		}
		else 
		{
			struct tm *the_time = localtime( &l->mExpires );
			bagspkt.WriteUInt32( the_time->tm_year );
			bagspkt.WriteUInt32( the_time->tm_mon );
			bagspkt.WriteUInt32( the_time->tm_mday );
			bagspkt.WriteUInt32( the_time->tm_hour );
			bagspkt.WriteUInt32( the_time->tm_min );
			bagspkt.WriteUInt32( the_time->tm_sec );
			bagspkt.WriteUInt32( 0 );
			bagspkt.WriteByte( l->mStatus );
		}
	}





	bagspkt.WriteUInt32( 0x393CCDE9 );  // Bank vault list hash.
	bagspkt.WriteUInt32( 0 );           // Number of bank vaults.
	Send( bagspkt, MSG_INVENTORY_BAGLIST );



	/*
	bagspkt.WriteUInt32( 6 );           // Number of bags.

	bagspkt.WriteUInt32( 0xDAA95362 );  // Bag hash. (counter ??)
	bagspkt.WriteUInt32( 1 );           // Bag number. (or index ??)
	bagspkt.WriteUInt32( 0x1769D5AF );  // Expiration date hash.
	bagspkt.WriteUInt16( 0x270F );      // Year.
    bagspkt.WriteUInt16( 9999 );        // Month.
	bagspkt.WriteUInt16( 1 );           // Day.
	bagspkt.WriteUInt16( 1 );           // Hour.
	bagspkt.WriteUInt16( 0 );           // Minute.
	bagspkt.WriteUInt16( 0 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 0 );             // Unknown.

	bagspkt.WriteUInt32( 0xDAA95362 + 1 );  // Bag hash. (counter ??)
	bagspkt.WriteUInt32(3 );           // Bag number. (or index ??)
	bagspkt.WriteUInt32( 0x1769D5AF );  // Expiration date hash.
	bagspkt.WriteUInt16( 110 );        // Year.
    bagspkt.WriteUInt16( 3 );           // Month.
	bagspkt.WriteUInt16( 17 );           // Day.
	bagspkt.WriteUInt16( 5 );           // Hour.
	bagspkt.WriteUInt16( 43 );           // Minute.
	bagspkt.WriteUInt16( 21 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 0 );             // Unknown.

	bagspkt.WriteUInt32( 0xDAA95362 + 2 );  // Bag hash. (counter ??)
	bagspkt.WriteUInt32( 4 );               // Index.
	bagspkt.WriteUInt32( 0x1769D5AF );      // Expiration date hash.
	bagspkt.WriteUInt16( 9999 );            // Year.
    bagspkt.WriteUInt16( 1 );           // Month.
	bagspkt.WriteUInt16( 1 );           // Day.
	bagspkt.WriteUInt16( 0 );           // Hour.
	bagspkt.WriteUInt16( 0 );           // Minute.
	bagspkt.WriteUInt16( 0 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 0 );             // Unknown.

	bagspkt.WriteUInt32( 0xDAA95362 + 3 );  // Bag hash. (counter ??)
	bagspkt.WriteUInt32( 5 );           // bag status
	bagspkt.WriteUInt32( 0x1769D5AF );  // Expiration date hash.
	bagspkt.WriteUInt16( 9999 );           // Year.
    bagspkt.WriteUInt16( 1 );           // Month.
	bagspkt.WriteUInt16( 1 );           // Day.
	bagspkt.WriteUInt16( 0 );           // Hour.
	bagspkt.WriteUInt16( 0 );           // Minute.
	bagspkt.WriteUInt16( 0 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 0 );             // Unknown.

	bagspkt.WriteUInt32( 0xDAA95362 + 4 );  // Bag hash (counter)
	bagspkt.WriteUInt32( 7 );               // Bag index.
	bagspkt.WriteUInt32( 0x1769D5AF );      // Expiration date hash.
	bagspkt.WriteUInt16( 9999 );            // Year.
    bagspkt.WriteUInt16( 1 );           // Month.
	bagspkt.WriteUInt16( 1 );           // Day.
	bagspkt.WriteUInt16( 0 );           // Hour.
	bagspkt.WriteUInt16( 0 );           // Minute.
	bagspkt.WriteUInt16( 0 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 0 );             // Locked status.

	bagspkt.WriteUInt32( 0xDAA95362 + 5 );  // Bag hash (counter)
	bagspkt.WriteUInt32( 2 );               // Bag index.
	bagspkt.WriteUInt32( 0x1769D5AF );      // Expiration date hash.
	bagspkt.WriteUInt16( 9999 );            // Year.
    bagspkt.WriteUInt16( 1 );           // Month.
	bagspkt.WriteUInt16( 1 );           // Day.
	bagspkt.WriteUInt16( 0 );           // Hour.
	bagspkt.WriteUInt16( 0 );           // Minute.
	bagspkt.WriteUInt16( 0 );           // Second.
	bagspkt.WriteUInt16( 0 );           // Millisecond.
	bagspkt.WriteByte( 1 );             // Unknown.
*/




	/* List of available skills. */
	Buffer skillspkt;
	skillspkt.WriteUInt32( 0x393C1A96 );
	skillspkt.WriteUInt32( 0 );
    // 4b - Skill Counter. (0x79b35362)
	// 4b - Skill ID.
	// 1b - Level
	Send( skillspkt, MSG_SKILLS_LIST );

	/* Square information. */
	Buffer squarepkt;
	squarepkt.WriteWideString( UTF16( mCharacter->mName ) ); // Charactername.
	squarepkt.WriteUInt32( 0x529E424F ); // Stagegroup Hash.
	squarepkt.WriteUInt32( 0x0330A106 ); // Stagegroup ID.
	squarepkt.WriteUInt16( 0 ); // Level.
	squarepkt.WriteUInt32( 0x393C107A ); // Unknown list hash.
	squarepkt.WriteUInt32( 0 );
	Send( squarepkt, 0x7260 );


	#define MSG_NPC_CREATE 0xB56C

// B56C

struct npc_data_t {
	uint32_t id;
	float px; 
	float py; 
	float pz;
	float dx; 
	float dy; 
	float dz;
} npc_list[] = {
	{37188954, 937.32f, 0.00f, 801.83f, -1.00f, 0.00f, -1.00f},
	{17702352, 641.55f, 0.00f, 758.76f, 1.00f, 0.00f, -1.00f},
	{40882796, 946.06f, 0.00f, 683.97f, -1.00f, 0.00f, -1.00f},
	{40997206, 734.80f, 0.00f, 898.42f, 0.00f, 0.00f, -1.00f},
	{1458109, 644.21f, 0.00f, 901.33f, 1.00f, 0.00f, -1.00f},
	{17510463, 644.60f, 0.00f, 674.94f, 1.00f, 0.00f, -1.00f},
	{41727393, 913.20f, 0.00f, 886.68f, -1.00f, 0.00f, -1.00f},
	{39511037, 768.22f, 0.00f, 924.65f, 1.00f, 0.00f, -1.00f},
	{49997863, 546.30f, 0.00f, 864.97f, 1.00f, 0.00f, -1.00f},
	{17618118, 844.56f, 0.00f, 655.10f, -1.00f, 0.00f, -1.00f},
	{13469290, 600.00f, 0.00f, 644.46f, 1.00f, 0.00f, -1.00f},
	{45143948, 757.21f, 0.00f, 660.49f, 1.00f, 0.00f, -1.00f},
	{7667169, 833.83f, 0.00f, 924.65f, -1.00f, 0.00f, -1.00f},
	{10698015, 777.00f, 0.00f, 976.03f, 1.00f, 0.00f, -1.00f},
	{26619883, 822.79f, 0.00f, 976.03f, -1.00f, 0.00f, -1.00f},
	{31130843, 583.49f, 0.00f, 909.34f, 1.00f, 0.00f, -1.00f},
	{15997574, 894.49f, 0.00f, 515.38f, 1.00f, 0.00f, -1.00f},
	{18197602, 866.34f, 0.00f, 894.02f, 0.00f, 0.00f, -1.00f},
	{29625892, 711.07f, 0.00f, 662.66f, 0.00f, 0.00f, -1.00f},
	{29625892, 895.81f, 0.00f, 666.49f, 0.00f, 0.00f, -1.00f},
	{29625892, 850.93f, 0.00f, 800.00f, 0.00f, 0.00f, -1.00f},
	{11617632, 820.56f, 0.00f, 655.10f, 0.00f, 0.00f, -1.00f},
	{63830058, 687.46f, 0.00f, 667.88f, 0.00f, 0.00f, -1.00f}
};

			for (int i = 0; i < 23; i++)
			{
				Buffer npc1pkt;
				npc1pkt.WriteUInt32( 10001 + i );
				npc1pkt.WriteUInt32( npc_list[i].id );
				npc1pkt.WriteUInt32( 5 );
				npc1pkt.WriteFloat( npc_list[i].px );
				npc1pkt.WriteFloat( npc_list[i].py );
				npc1pkt.WriteFloat( npc_list[i].pz );
				npc1pkt.WriteFloat( npc_list[i].dx );
				npc1pkt.WriteFloat( npc_list[i].dy );
				npc1pkt.WriteFloat( npc_list[i].dz );
				Send( npc1pkt, MSG_NPC_CREATE );
			}

//------------------------------------------------------------
//-----------       Created with 010 Editor        -----------
//------         www.sweetscape.com/010editor/          ------
//
// File    : C:\Users\Dennis\Desktop\test3.bin
// Address : 45 (0x2D)
// Size    : 76 (0x4C)
//------------------------------------------------------------
unsigned char test1[74] = {
    0x69, 0xE0, 0x8E, 0xCE, 0x0B, 0x00,
    0x00, 0x00, 0xEB, 0x03, 0x00, 0x00, 0x5F, 0x00,
    0xEF, 0x03, 0x00, 0x00, 0xFF, 0x0F, 0xF3, 0x03,
    0x00, 0x00, 0xFE, 0x1F, 0xF6, 0x03, 0x00, 0x00,
    0xD5, 0x0F, 0xF7, 0x03, 0x00, 0x00, 0xFF, 0x0F,
    0xF9, 0x03, 0x00, 0x00, 0x00, 0x01, 0xFE, 0x03,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x00, 0x00,
    0xFF, 0x0F, 0x06, 0x04, 0x00, 0x00, 0xA0, 0x0F,
    0x07, 0x04, 0x00, 0x00, 0xFE, 0x0F, 0x08, 0x04,
    0x00, 0x00, 0x3F, 0x00 
};
Send( Buffer( (char *)test1, 74 ), 0xA80E );

//------------------------------------------------------------
//-----------       Created with 010 Editor        -----------
//------         www.sweetscape.com/010editor/          ------
//
// File    : C:\Users\Dennis\Desktop\test3.bin
// Address : 31 (0x1F)
// Size    : 10 (0xA)
//------------------------------------------------------------
unsigned char test2[10] = {
    0x00, 0x00, 0x00, 0x00, 0xEE, 0x03, 0x00, 0x00 
};
Send( Buffer( (char *)test2, 8 ), 0xA918 );

	// Send quests...
}



void PlayerSession::SendCharacterList()
{
	Buffer charapkt;
	charapkt.WriteUInt32( mCharacter->mId );
	charapkt.WriteUInt32( 0 );
	charapkt.WriteWideString( UTF16( mCharacter->mName ) );
	charapkt.WriteUInt16( mCharacter->mLevel );
	charapkt.WriteUInt16( mCharacter->mPvpLevel );
	charapkt.WriteUInt16( mCharacter->mWarLevel );
	charapkt.WriteUInt16( mCharacter->mRebirthLevel );
	charapkt.WriteUInt16( mCharacter->mRebirthCount );
/*
	chara.WriteFloat(position.x);
	chara.WriteFloat(position.y);
	chara.WriteFloat(position.z);
	chara.WriteFloat(direction.x);
	chara.WriteFloat(direction.y);
	chara.WriteFloat(direction.z);
*/
	charapkt.WriteFloat( 1200.0f );
	charapkt.WriteFloat( 0.0f );
	charapkt.WriteFloat( 610.0f );
	charapkt.WriteFloat( mDirection.x );
	charapkt.WriteFloat( mDirection.y );
	charapkt.WriteFloat( -1 );
	charapkt.WriteFloat( 100.0f );
	charapkt.WriteUInt16( (uint16_t)1 );

	/* Equipment */
	charapkt.WriteUInt32( HASH_LIST_EQUIPMENT );
	charapkt.WriteUInt32( 0 );

	/* Passive items. */
	charapkt.WriteUInt32( HASH_LIST_PASSIVEITEMS );
	charapkt.WriteUInt32( 0 );

	/* stateflags */
	charapkt.WriteUInt32( HASH_LIST_STATEFLAGS );
	charapkt.WriteUInt32( 0 );

	charapkt.WriteByte( 0 ); // shopping?

	/* Stage licenses. */
	charapkt.WriteUInt32( HASH_LIST_STAGELICENSES );
	charapkt.WriteUInt32( 0 );

	charapkt.WriteByte( 0 );   // Lives.
	charapkt.WriteByte( 0 );   // Bonuslife.
	charapkt.WriteUInt32( 0 ); // Stageflags.


	unsigned char hexData[18] = {
		0x55, 0xD5, 0x69, 0x17, 0xC3, 0x07, 0x05, 0x00,
		0x10, 0x00, 0x01, 0x00, 0x1E, 0x00, 0x00, 0x00,
		0x00, 0x00 
	};
	charapkt.Write( hexData, 18 );


	Send( charapkt, 0x831f );
}