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
#include <database.h>
#include <stdio.h>
#include <log.h>

MYSQL *DB::mConn;

static char sql[1024];

/* Opens a connection with the MySQL server. */
bool DB::Connect( const char *host, const char *user, const char *passwd, const char *db, uint16_t port )
{
	if ( mConn != NULL )
	{
		if ( mysql_ping( mConn ) != 0 )
		{
			mysql_close( mConn );
		}
		else return true;
	}

	mConn = mysql_init( NULL );
	if ( mysql_real_connect( mConn, host, user, passwd, db, port, NULL, 0 ) == NULL )
	{
		return false;
	}
	return true;
}

/* Gets all the characters that belong to the specified account. */
uint32_t DB::Character_GetList( uint32_t account_id, CharacterList &list )
{
	if ( account_id == 0 )
		return 0;

	sprintf( sql, "SELECT `id` FROM `sol_characters` WHERE `account_id` = %d", account_id, MAX_CHARACTERS );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return 0;
	}

	MYSQL_RES *result = mysql_store_result( mConn );
	if ( result == NULL )
	{
		DB::LogError();
		return 0;
	}

	uint32_t numchars = (uint32_t)mysql_num_rows( result );
	if ( numchars == 0 )
	{
		mysql_free_result( result );
		return 0;
	}

	MYSQL_ROW row = NULL;
	for ( uint32_t i = 0; row = mysql_fetch_row( result ); i++ )
	{
		CharacterData *chara = Character_Load( atoi( row[0] ), NULL );
		if ( chara != NULL )
		{
			list.push_back( chara );
		}
	}
	mysql_free_result( result );
	return numchars;
}

/* Deletes the character with the specified ID. */
void DB::Character_Delete( uint32_t char_id )
{
	sprintf( sql, "DELETE FROM sol_characters WHERE id = %d", char_id );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
	}
}

/* Retrieves the details of the character with the specified ID. */
CharacterData *DB::Character_Load( uint32_t char_id, CharacterData *info )
{
	sprintf( sql, "SELECT * FROM `sol_characters` WHERE `id` = %d", char_id );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return NULL;
	}

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res == NULL || mysql_num_rows( res ) == 0 )
	{
		mysql_free_result( res );
		return NULL;
	}

	MYSQL_ROW row = mysql_fetch_row( res );
	if ( row != NULL )
	{
		if ( info == NULL )
		{
			info = new CharacterData();
		}

		strcpy( info->mName, row[2] );

		info->mId             = atoi( row[0]  );
		info->mClassId        = atoi( row[3]  );
		info->mLevel          = atoi( row[4]  );
		info->mExperience     = atoi( row[5]  );
		info->mPvpLevel       = atoi( row[6]  );
		info->mPvpExperience  = atoi( row[7]  );
		info->mWarLevel       = atoi( row[8]  );
		info->mWarExperience  = atoi( row[9]  );
		info->mRebirthLevel   = atoi( row[10] );
		info->mRebirthCount   = atoi( row[11] );
		info->mLastPlayed     = atol( row[12] );
		info->mEquipmentCount = 0;

		#if defined( _SQUARE )
		info->mMoney          = atoi( row[13] );
		info->mBank.mMoney    = atoi( row[14] );

		/* Load items. */
		Character_LoadInventory( info );
		Character_LoadBank( info );
		#endif

		mysql_free_result( res );
	}
	return info;
}

/* Loads the inventory of the specified character. */
void DB::Character_LoadInventory( CharacterData *c )
{
#if defined( _SQUARE )
	if ( c == NULL || c->mId == 0 ) return;

	c->mBagLicenses = new std::vector<BagLicense *>();

	sprintf( sql, "SELECT * FROM bags WHERE char_id = %u", c->mId );
	if ( mysql_query( mConn, sql ) == 0 )
	{
		MYSQL_RES *res = mysql_store_result( mConn );
		if ( res != NULL )
		{
			MYSQL_ROW row;
			while ( row = mysql_fetch_row( res ) )
			{
				BagLicense *license = (BagLicense *)malloc( sizeof( BagLicense ) );

				license->mId      = atol( row[0] );
				license->mIndex   = atoi( row[2] );
				license->mStatus  = atoi( row[3] );
				license->mExpires = atol( row[4] );

				c->mBagLicenses->push_back( license );
			}

			mysql_free_result( res );
		}
	}

	sprintf( sql, "SELECT * FROM items WHERE type = 0 AND char_id = %u", c->mId );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return;
	}

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res == NULL )
	{
		DB::LogError();
		return;
	}

	MYSQL_ROW row;
	while ( row = mysql_fetch_row( res ) )
	{
		ItemInfo *i = &c->mBags[atoi( row[4] )][atoi( row[5] )];

		i->mId     = atoi( row[0] );
		i->mItemId = atoi( row[2] );
		i->mAmount = atoi( row[6] );
	}
	mysql_free_result( res );
#endif
}

/* Loads the bank of the specified character. */
void DB::Character_LoadBank( CharacterData *c )
{
#if defined( _SQUARE )
	if ( c == NULL || c->mId == 0 ) return;

	sprintf( sql, "SELECT * FROM items WHERE type = 1 AND char_id = %u", c->mId );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return;
	}

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res == NULL )
	{
		DB::LogError();
		return;
	}

	MYSQL_ROW row;
	while ( row = mysql_fetch_row( res ) )
	{
		ItemInfo *i = &c->mBank.mBags[atoi( row[4] )][atoi( row[5] )];

		i->mId     = atoi( row[0] );
		i->mItemId = atoi( row[2] );
		i->mAmount = atoi( row[6] );
	}
	mysql_free_result( res );
#endif
}

/* Checks if a characters with the specified name exists. */
bool DB::Character_Exists( const char *name )
{
	char *p_sql = sql;
	p_sql += sprintf( p_sql, "SELECT COUNT(`id`) FROM `sol_characters` WHERE `name` = '" );
	p_sql += mysql_real_escape_string( mConn, p_sql, name, strlen( name ) );
	p_sql += sprintf( p_sql, "'" );

	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return false;
	}

	bool exists = false;

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res != NULL )
	{
		if ( mysql_num_rows( res ) == 1 )
		{
			MYSQL_ROW row = mysql_fetch_row( res );
			if ( row != 0 )
			{
				exists = ( atoi( row[0] ) > 0 );
			}
		}
		mysql_free_result( res );
	}
	return exists;
}

/* Creates a new character. */
uint32_t DB::Character_Create( uint32_t account_id, uint32_t class_id, const char *name )
{
	char *p_sql = sql;
	p_sql += sprintf( p_sql, "INSERT INTO `sol_characters` (`account_id`, `class`, `name`) VALUES (%d, %d, '", account_id, class_id );
	p_sql += mysql_real_escape_string( mConn, p_sql, name, strlen( name ) );
	p_sql += sprintf( p_sql, "')" );

	if ( mysql_query( mConn, sql ) != 0 )
	{
		return 0;
	}
	return (uint32_t)mysql_insert_id( mConn );
}

/* Loads the account with the specified name. */
AccountInfo *DB::Account_Load( const char *account_name, AccountInfo *account, bool load_charlist )
{
	/* Generate the query. */
	char *p_sql = sql;
	p_sql += sprintf( p_sql, "SELECT id FROM `sol_accounts` WHERE `name` = '" );
	p_sql += mysql_real_escape_string( mConn, p_sql, account_name, strlen( account_name ) );
	p_sql += sprintf( p_sql, "';" );

	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return NULL;
	}

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res != NULL )
	{
		if ( mysql_num_rows( res ) == 0 )
		{
			mysql_free_result( res );
			return NULL;
		}

		MYSQL_ROW row = mysql_fetch_row( res );
		uint32_t account_id = atoi( row[0] );

		mysql_free_result( res );
		return Account_Load( account_id, account, load_charlist );
	}
	return NULL;
}


/* Loads the account with the specified id. */
AccountInfo *DB::Account_Load( uint32_t account_id, AccountInfo *account, bool load_charlist )
{
	if ( account == NULL )
	{
		account = (AccountInfo *)malloc( sizeof( AccountInfo ) );
		if ( account == NULL )
		{
			return NULL;
		}
	}
	
	/* Generate the query. */
	sprintf( sql, "SELECT `name`, `max_chars`, `passwd`, `status`, `gmlevel` FROM `sol_accounts` WHERE `id` = %d", account_id );
	if ( mysql_query( mConn, sql ) != 0 )
	{
		DB::LogError();
		return NULL;
	}

	MYSQL_RES *res = mysql_store_result( mConn );
	if ( res == NULL )
	{
		DB::LogError();
		return NULL;
	}

	MYSQL_ROW row = mysql_fetch_row( res );
	if ( row != NULL )
	{
		/* Copy the data to the account struct. */
		account->mId       = account_id;
		account->mMaxChars = atoi( row[1] );
		account->mStatus   = atoi( row[3] );
		account->mGmLevel  = atoi( row[4] );

		strcpy( account->mName, row[0] );
		strcpy( account->mPassword, row[2] );
		mysql_free_result( res );

		/* Get the available character licences for this account. */
		sprintf( sql, "SELECT class_id FROM sol_character_licenses WHERE account_id = %d", account->mId );
		if ( mysql_query( mConn, sql ) == 0 )
		{
			res = mysql_store_result( mConn );
			if ( res != NULL )
			{
				account->mLicenseCount = (uint32_t)mysql_num_rows( res );
				for ( uint32_t i = 0; row = mysql_fetch_row( res ); i++ )
				{
					account->mLicenses[i] = atoi( row[0] );
				}

				mysql_free_result( res );
			}
		}

		/* Get the characters. */
		if ( load_charlist )
		{
			DB::Character_GetList( account->mId, account->mCharacters );
		}
	}
	else mysql_free_result( res );
	
	return account;
}

/* Writes a MySQL error to the error log. */
void DB::LogError()
{
	ErrorLog.Write( "SQL Error: [%d] %s\n", E_ERROR, mysql_errno( mConn ), mysql_error( mConn ) );
}
