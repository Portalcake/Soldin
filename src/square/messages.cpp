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
#include <square.h>

extern GatewayClient *g_gateway;

/* Receives a session key from the client and tries to lookup the details for that session. */
void PlayerSession::Msg_Load_Authenticate( Buffer &packet )
{
	const char *session_key = UTF8( packet.ReadWideString() );
	#if defined(_DEBUG)
	DebugLog.Write( "[%d][CLIENT] Received session key '%s', authenticating...\n", E_INFO, mSessionId, session_key );
	#endif

	g_gateway->GetSessionInfo( mSessionId, session_key );
}

/* When the client sends the loading progress reply with the previous
 * load progress value. Not always send by the client when its at 100%,
 * so this cannot be used to tell wether the clients is done loading. */
void PlayerSession::Msg_Load_Progress( Buffer &packet )
{
	float progress = packet.ReadFloat();

	Buffer progresspkt;
	progresspkt.WriteWideString( UTF16( mCharacter->mName ) );
	progresspkt.WriteFloat( mProgress );

	Send( progresspkt, MSG_LOAD_PROGRESS );
	mProgress = progress;
}

/* Client has finished loading, move the client to the square stage. */
void PlayerSession::Msg_Load_Done( Buffer &buffer )
{
	SendCharacterList();

	/* StateBundle data?? ??? */
	/*
			Buffer changestagepkt;
			changestagepkt.WriteUInt32(player->GetCharacter()->id);
			changestagepkt.WriteUInt32(m_hash); // Stage ID
			changestagepkt.WriteUInt32(2); // Level.
			changestagepkt.WriteUInt32(0); // ID of stage change initiator.
			Send(changestagepkt, MSG_STAGE_CHANGE);*/

	mStage = Square::GetStage();

	int result = 0;
	if ( ( result = mStage->Join( this ) ) != STERR_NONE )
	{
		mEOF = true;
	}
}

/* Sends a list of all the items in the characters bags. */
void PlayerSession::Msg_Inventory_GetBagItems( Buffer &packet )
{
	Buffer listpkt;
	listpkt.WriteUInt32( HASH_LIST_BAGITEMS );
	listpkt.WriteUInt32( 0 );

	Send( listpkt, MSG_INVENTORY_GETBAGITEMS );
}

/* Sends a list of all the items in the characters bank. */
void PlayerSession::Msg_Inventory_GetBankItems( Buffer &packet )
{
	Buffer listpkt;
	listpkt.WriteUInt32( HASH_LIST_BANKITEMS );
	listpkt.WriteUInt32( 0 );
	/*
	out.Add_uint32(0x13d35362); // ItemSlot identifier/Counter
	out.Add_uint32(0x01566d86); // Id
	out.Add_uint32(0x613e5dca); // ItemPosition identifier
	out.Add_uint8(0x00); // Bag
	out.Add_uint8(0x01); // Position
	out.Add_uint8(0x23); // Stacked
	out.Add_uint64(0); // Instance
	*/
	Send( listpkt, MSG_INVENTORY_GETBANKITEMS );
}

/* Gets the quickbar layout. */
void PlayerSession::Msg_Skills_GetQuickSlots( Buffer &packet )
{
	Buffer listpkt;
	listpkt.WriteUInt32( HASH_LIST_QUICKSLOTS );
	listpkt.WriteUInt32( 0 );
	/*
	// uint32 counter = 0xdc2e5362
	// uint32 hash ?
	// uint8 quickslotset?
	// uint8 quickslot?
	// uint64 instance? [0 for skills]
	*/
	Send( listpkt, MSG_SKILLS_GETQUICKSLOTS );
}

/* Handle character movement. */
void PlayerSession::Msg_Character_Move( Buffer &packet )
{
	struct move_t 
	{
		uint32_t mCommand;
		uint32_t mDir;
	} 
	*action = (move_t *)( packet.Content() + 6 );

	switch ( action->mCommand )
	{
		case 0:	StartMovement( action->mDir, false );      break;
		case 2:	StopMovement();                            break;
		case 1: /* TODO: Implement the ability to dash. */ break;
	}
}

/* Handle chat messages. */
void PlayerSession::Msg_Chat_Message( Buffer &packet )
{
	uint32_t type = packet.ReadUInt32();
	const wchar_t *message = packet.ReadWideString();

	const char *str = UTF8( message );
	if ( str[0] == '#' )
	{
		/*if ( _stricmp(str + 1, "update") == 0 )
		{
			message = UTF16( "[GM] Sending position update." );

			SetAction( ( mMoving ? ACT_RUN : ACT_IDLE ) );
		}*/
	}

	/* Send the chat message. */
	Buffer chatpkt;
	chatpkt.WriteUInt32( mCharacter->mId );
	chatpkt.WriteUInt32( type );
	chatpkt.WriteWideString( message );
	//Send(chatpkt, MSG_CHAT_MESSAGE);
	mStage->Send( chatpkt, MSG_CHAT_MESSAGE );
}



void PlayerSession::Msg_Shop_Enter( Buffer &packet ) 
{
	uint32_t shop_id = packet.ReadUInt32();

	Buffer response;
	response.WriteUInt32( mCharacter->mId );
	response.WriteUInt32( shop_id );
	response.WriteUInt32( 0 );

	mStage->Send( response, MSG_SHOP_ENTER );
}

void PlayerSession::Msg_Shop_Leave( Buffer &packet )
{
	Buffer response;
	response.WriteUInt32( mCharacter->mId );
	response.WriteFloat( mPosition.x );
	response.WriteFloat( mPosition.y );
	response.WriteFloat( mPosition.z );
	response.WriteFloat( mDirection.x );
	response.WriteFloat( mDirection.y );
	response.WriteFloat( mDirection.z );

	mStage->Send( response, MSG_SHOP_LEAVE );
}

void PlayerSession::Msg_Shop_Buy( Buffer &packet )
{
}

void PlayerSession::Msg_Shop_Sell( Buffer &packet )
{
}

/* Deposits money into the bank. */
void PlayerSession::Msg_Bank_Deposit( Buffer &packet )
{
	uint32_t amount = packet.ReadUInt32();
	if ( amount <= mCharacter->mMoney )
	{
		mCharacter->mMoney       -= amount;
		mCharacter->mBank.mMoney += amount;
	}

	Buffer response;
	response.WriteUInt32( amount );
	response.WriteUInt32( 0 );
	Send( response, MSG_BANK_DEPOSIT );
}

/* Withdraws money from the bank. */
void PlayerSession::Msg_Bank_Withdraw( Buffer &packet )
{
	uint32_t amount = packet.ReadUInt32();
	if ( amount <= mCharacter->mBank.mMoney )
	{
		mCharacter->mBank.mMoney -= amount;
		mCharacter->mMoney       += amount;
	}

	Buffer response;
	response.WriteUInt32( amount );
	response.WriteUInt32( 0 );
	Send( response, MSG_BANK_WITHDRAW );
}

/* Moves items from the players inventory bags to their bank bags. */
void PlayerSession::Msg_Bank_Move( Buffer &packet )
{
	packet.ReadUInt32();
	uint16_t  src_bag    = packet.ReadUInt16();
	uint16_t  src_index  = packet.ReadUInt16();
	ItemInfo *source     = &mCharacter->mBags[src_bag][src_index];

	packet.ReadUInt32();
	uint16_t  dest_bag   = packet.ReadUInt16();
	uint16_t  dest_index = packet.ReadUInt16();
	ItemInfo *dest       = &mCharacter->mBank.mBags[dest_bag][dest_index];


	if ( source->mItemId != 0 )
	{
	
		if ( dest->mItemId != 0 )
		{
			/* Swap destination and source. */
			ItemInfo temp;
			memcpy( &temp, source, sizeof( ItemInfo ) );

			memcpy( source, dest, sizeof( ItemInfo ) );
			memcpy( dest, &temp, sizeof( ItemInfo ) );
		}
		else
		{
			/* Copy destination to source and clear the source. */
			memcpy( dest, source, sizeof( ItemInfo ) );
			memset( source, 0, sizeof( ItemInfo ) );
		}
	}
}
