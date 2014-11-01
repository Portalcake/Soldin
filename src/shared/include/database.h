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
#ifndef __SOLIN_DATABASE_H__
#define __SOLIN_DATABASE_H__

#include <winsock2.h> 
#include <mysql.h>
#include <shared.h>
#include <character.h>
#include <account.h>

class DB {
public:
	static bool Connect( const char *host, const char *user, const char *passwd, const char *db, uint16_t port );

    /*  Character management. */
	static uint32_t       Character_GetList( uint32_t account_id, CharacterList &list );
	static void           Character_Delete( uint32_t char_id );
	static CharacterData *Character_Load( uint32_t char_id, CharacterData *info );
	static bool           Character_Exists( const char *name );
	static uint32_t       Character_Create( uint32_t account_id, uint32_t class_id, const char *name );
	//static uint32_t       Character_GetEquipment( uint32_t char_id, EquipmentInfo *list );
	static void           Character_LoadInventory( CharacterData *c );
	static void           Character_LoadBank( CharacterData *c );


	/* Account management. */
	static AccountInfo   *Account_Load( const char *account_name, AccountInfo *account, bool load_charlist = true );
	static AccountInfo   *Account_Load( uint32_t account_id, AccountInfo *account, bool load_charlist = true );
	//static void           Account_Delete( uint32_t account_id );

	/* Error handling. */
	static void LogError();

private:
	static MYSQL *mConn;
};

#endif /* __SOLIN_DATABASE_H__ */
