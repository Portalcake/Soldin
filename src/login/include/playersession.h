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
#include <sessionmanager.h>
#include <squaremanager.h>
#include <database.h>
#include <messages.h>

/* Hashes. */
#define HASH_LIST_CHARACTERS       0x393CAF2B
#define HASH_LIST_CHARLICENCES     0x393C883E
#define HASH_LIST_EQUIPMENT        0x393CD166
#define HASH_LIST_STAGELICENCES    0x393CE8F3
#define HASH_LIST_SQUARES          0x393CDC25
#define HASH_OBJ_CHARACTER         0xBC715362
#define HASH_OBJ_NEWCHARACTER      0xBC713DDA // not a counter.
#define HASH_OBJ_EQUIPMENT         0x13D35362
#define HASH_OBJ_SQUARE            0xA7AD5362
#define HASH_DATETIME_CONNECTED    0x1769F8F2
#define HASH_DATETIME_LASTPLAYED   0x17691AC9

/* Error codes. */
#define ERR_NONE                       0
#define ERR_LOGIN_NOTFOUND             1 /* This user does not exist. */
#define ERR_LOGIN_INVALIDPASSWD        2 /* Incorrect password. */
#define ERR_LOGIN_ACCDELETED           3 /* This account was deleted. */
#define ERR_LOGIN_ACCBLOCKED           4 /* Your account has been blocked. */
#define ERR_LOGIN_ALREADYCONLOBBY      5 /* You are already connected to the Lobby. */
#define ERR_LOGIN_ALREADYCONSERVER     6 /* You are already connected to the game server. */
#define ERR_CHARCREATE_NAMETAKEN       1 /* The character name is already in use. */
#define ERR_CHARCREATE_DELETED         2 /* This name cannot be used because it's deleted. */
#define ERR_CHARCREATE_INCORRECT       3 /* Incorrect user name. */
#define ERR_CHARCREATE_FAILED          4 /* Failed to create a character. */
#define ERR_CHARCREATE_MIGRATIONPERIOD 6 /* You cannot create character while server migration request period. */
#define ERR_DELCHAR_NOTFOUND           1 /* This character does not exist. */
#define ERR_DELCHAR_GUILDMASTER        2 /* Guild Master cannot delete character. */
#define ERR_DELCHAR_GUILDMASTERLEAVE   3 /* Guild Master cannot delete character. Leave your guild first. */
#define ERR_DELCHAR_WRONGIDENT         4 /* It's wrong identification number. You cannot delete your character. */
#define ERR_SELCHAR_CANCEL             1 /* NOTE: client will silently move back to the character list. */
#define ERR_SELCHAR_PROHIBITED         2
#define ERR_SELSQUARE_DUPLICATENAMES   1 /* There is a room using the same name. */
#define ERR_SELSQUARE_STAGENOTFOUND2   2 /* Failed to find the stage bearing the same name. */
#define ERR_SELSQUARE_STAGENOTFOUND    3 /* Failed to find the stage. */
#define ERR_SELSQUARE_NOACCESS         4 /* You cannot access the stage. */
#define ERR_SELSQUARE_FULL             5 /* You cannot enter the stage because it's full. */
#define ERR_SELSQUARE_NOTAUTHORIZED    6 /* You are not authorized to access. */
#define ERR_SELSQUARE_NOTFOUND         7 /* Unable to find server. */
#define ERR_SELSQUARE_UNKNOWNERROR     8 /* Unknown error. */

#define ST_INLOBBY 0
#define ST_INGAME 1

/* Represents a client that has connected to the server */
class PlayerSession: public Session {
public:
	PlayerSession(Socket *socket);
	~PlayerSession();

	void              Update();
	void              Process( Buffer &buffer );
    void              Send( Buffer &buffer, uint16_t cmd, uint16_t type = 0x55E0 );
	inline Socket    *GetSocket()       const { return mSocket; }
	inline uint32_t   GetCharacterID()  const { return ( mCharacter != NULL ) ? mCharacter->mId : 0; }
	inline uint32_t   GetAccountID()    const { return ( mAccount != NULL ) ? mAccount->mId : 0; }
	inline uint8_t    GetStatus()       const { return mStatus; }
	inline bool       IsConnected()     const { return mSocket->Connected(); }
	inline bool       IsAuthenticated() const { return ( mAuthenticated && ( mAccount != NULL ) ); }

private:
	void Unsupported(Buffer &packet);

	/* Message handlers. */
	void Msg_ClientHash       ( Buffer &packet );
	void Msg_Login            ( Buffer &packet );
	void Msg_Logout           ( Buffer &packet );
	void Msg_CharacterCreate  ( Buffer &packet );
	void Msg_CharacterDelete  ( Buffer &packet );
	void Msg_CharacterSelect  ( Buffer &packet );
	void Msg_CharacterDeselect( Buffer &packet );
	void Msg_SquareSelect     ( Buffer &packet );
	void Msg_SquareList       ( Buffer &packet );
	void Msg_Ping             ( Buffer &packet );
	
	void SendCharacterList();
	void SendSquareList();

	Socket        *mSocket;
	Buffer         mBufferIn;
	Buffer         mBufferOut;
	CharacterData *mCharacter;
	AccountInfo   *mAccount;
	bool           mAuthenticated;
	Square        *mSquare;
	uint8_t        mStatus;
};

#endif /* __SOLDIN_PLAYERSESSION_H__ */
