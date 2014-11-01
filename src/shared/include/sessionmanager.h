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
#ifndef __SOLDIN_SESSIONMANAGER_H__
#define __SOLDIN_SESSIONMANAGER_H__

#include <shared.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define INVALID_SESSION -1
#define MAX_SESSIONS    200

/* Session types. */
#define SESS_NONE       0
#define SESS_GATEWAY    1
#define SESS_SQUARE     2
#define SESS_USER       3

class Session
{
public:
	uint8_t     mType;
	int         mSessionId;
	char        mSessionKey[9];
	bool        mEOF;
	const char *mName;

	/* Initializes the session by generating a random key. */
	void Initialize()
	{
		sprintf( &mSessionKey[0], "%8X", time( NULL ) + rand() );
		mSessionKey[8] = 0;
	}
};

/* Represents the session list. */
class SessionManager
{
public:
	static int  Create( byte type, Session *object );
	static bool Destroy( int index );
	//static void Initialize() { memset( &mSessions, 0, sizeof( Session * ) * MAX_SESSIONS ); }

	/* Finds the session with the specified key. */
	template < class _T >
	static _T *Find( const char *session_key )
	{
		for ( int i = 0; i < MAX_SESSIONS; i++ )
			if ( strcmp( mSessions[i]->mSessionKey, session_key ) == 0 )
				return (_T *)mSessions[i];
		
		return NULL;
	}

	/* Gets the session object at the specified index. */
	template < class _T >
	static _T* At( int index )
	{
		if ( index >= MAX_SESSIONS )
			return NULL;

		return (_T *)mSessions[index];
	}

	/* Finds the session with the specified name. */
	template < class _T >
	static _T *FindByName( const char *name )
	{
		for ( int i = 0; i < MAX_SESSIONS; i++ )
			if ( mSessions[i]->mName != NULL && _stricmp( mSessions[i]->mName, name ) == 0 )
				return (_T *)mSessions[i];

		return NULL;
	}

private:
	static Session *mSessions[MAX_SESSIONS];
	static size_t   mSessionCount;
};

#endif /* __SOLDIN_SESSIONMANAGER_H__ */
