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
#include <sessionmanager.h>

Session *SessionManager::mSessions[MAX_SESSIONS];
size_t   SessionManager::mSessionCount = 0;

/* Registers the specified session. */
int SessionManager::Create( uint8_t type, Session *object )
{
	for ( int i = 0; i < MAX_SESSIONS; i++ )
	{
		if ( mSessions[i] != NULL )
			continue;

		mSessions[i]             = object;
		mSessions[i]->mType      = type;
		mSessions[i]->mSessionId = i;
		mSessions[i]->mName      = NULL;

		#ifdef _GATEWAY
		/* Only the gateway needs to generate a session key. */
		mSessions[i]->Initialize();
		#endif

		mSessionCount++;
		return i;
	}
	return INVALID_SESSION;
}

/* Removes the session at the specified index. */
bool SessionManager::Destroy( int index )
{
	if ( index >= MAX_SESSIONS )
		return false;

	if ( mSessions[index] != NULL )
	{
		mSessions[index] = NULL;
		mSessionCount--;

		return true;
	}
	return false;
}
