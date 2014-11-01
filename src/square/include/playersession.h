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
#ifndef __SOLDIN_PLAYERSESSION_H__
#define __SOLDIN_PLAYERSESSION_H__

#include <socket.h>
#include <buffer.h>
#include <sessionmanager.h>
#include <database.h>

/* Packet command ID's. */
#define MSG_CHARACTER_INFO			0x3DDA
#define MSG_CHARACTER_MOVE			0xE28D
#define MSG_CHARACTER_SETACTION		0x4228
#define MSG_LOAD_ENCRYPTIONKEY		0x5e71 // send only by the server.
#define MSG_LOAD_AUTHENTICATE		0x7260 // send only by the client.
#define MSG_LOAD_PROGRESS			0x81B6
#define MSG_LOAD_DONE				0xB98B
#define MSG_INVENTORY_GETBAGITEMS	0x2CF3
#define MSG_INVENTORY_BAGLIST		0xFA8D
#define MSG_INVENTORY_GETBANKITEMS	0x5190
#define MSG_CHAT_MESSAGE			0x6FB9
#define MSG_SKILLS_GETQUICKSLOTS	0x0E0F
#define MSG_SKILLS_LIST				0x7414
#define MSG_SHOP_ENTER              0x1091
#define MSG_SHOP_LEAVE              0xD0F0
#define MSG_SHOP_BUY                0x1208
#define MSG_SHOP_SELL               0xABB3
#define MSG_BANK_DEPOSIT            0xF3CB
#define MSG_BANK_WITHDRAW           0x3D0B
#define MSG_BANK_MOVE               0xE29D

#define HASH_UNKNOWN_LIST1          0x393c2708
#define HASH_UNKNOWN_LIST2          0x393ce1cc
#define HASH_UNKNOWN_LIST3          0x393ccd39

/* List hashes. */
#define HASH_LIST_BAGITEMS			0x393CEE31
#define HASH_LIST_BANKITEMS			0x393C882D
#define HASH_LIST_QUICKSLOTS		0x393CC9EC
#define HASH_LIST_EQUIPMENT			0x393CD166
#define HASH_LIST_PASSIVEITEMS		0x393C9E59
#define HASH_LIST_STATEFLAGS		0x393C1276
#define HASH_LIST_STAGELICENSES		0x393C61D4

/* Action hashes. */
#define ACT_IDLE	0x01327338
#define ACT_RUN		0x00004e0d
#define ACT_DASH	0x0002cbf3

class Stage;
class PlayerSession: public Session
{
public:
	PlayerSession( Socket *socket );
	~PlayerSession();

	void           Update();
	void           Process( Buffer &buffer );
    void           Send( Buffer &buffer, uint16_t cmd, uint16_t type = 0x55E0 );
	void           SendBoardMessage( const char *from, const char *message );
	void           SetAction( uint32_t action );
	void           SetEncryptionKey( uint32_t key );
	void           LoadCharacter( uint32_t char_id, uint32_t account_id );

	inline bool	   Connected()    { return mSocket->Connected(); }
	inline Socket *GetSocket()    { return mSocket; }
	CharacterInfo *GetCharacter() { return mCharacter; }

private:
	Socket        *mSocket;
	Buffer         mBufferIn;
	Buffer         mBufferOut;
	float		   mProgress;
	Vector3		   mPosition;
	Vector3		   mDirection;
	AccountInfo   *mAccount;
	CharacterInfo *mCharacter;
	Stage         *mStage;

	/* Movement */
	//uint32_t last_move_tick;
	//uint32_t dash_tick;

	
	bool     mMoving;
	uint32_t mMoveStartTick;
	uint8_t  mMoveDirection;
	//bool     is_dash;
	
	void StartMovement( uint8_t dir, bool dash );
	void StopMovement();
	void CalculatePosition();
	//void Move( uint32_t tick );

	void Unsupported( Buffer &packet );

	/* Packet handlers. */
	void Msg_Character_Move( Buffer &packet );
	void Msg_Load_Authenticate( Buffer &buffer );
	void Msg_Load_Progress( Buffer &buffer );
	void Msg_Load_Done( Buffer &buffer );
	void Msg_Inventory_GetBagItems( Buffer &buffer );
	void Msg_Inventory_GetBankItems( Buffer &buffer );
	void Msg_Skills_GetQuickSlots( Buffer &packet );
	void Msg_Chat_Message( Buffer &packet );

	void Msg_Shop_Enter( Buffer &packet );
	void Msg_Shop_Leave( Buffer &packet );
	void Msg_Shop_Buy( Buffer &packet );
	void Msg_Shop_Sell( Buffer &packet );

	void Msg_Bank_Deposit( Buffer &packet );
	void Msg_Bank_Withdraw( Buffer &packet );
	void Msg_Bank_Move( Buffer &packet );

	void SendCharacterList();
	void SendCharacterInfo();
};

#endif /* __SOLDIN_PLAYERSESSION_H__ */
