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
#include <squaremanager.h>

Square   SquareManager::mSquareList[MAX_SQUARES];
uint32_t SquareManager::mSquareCount = 0;

/* Adds the specified square to the list. */
int SquareManager::Add( const char *name, uint32_t hostaddr, uint16_t port, uint32_t capacity, SquareSession *session )
{
	if ( !session || !session->IsConnected() || mSquareCount == MAX_SQUARES )
		return -1;

	for ( int i = 0; i < MAX_SQUARES; i++ )
	{
		if ( mSquareList[i].mSession == NULL )
		{
			strcpy( mSquareList[i].mName, name );

			mSquareList[i].mHostAddr.S_un.S_addr = hostaddr;
			mSquareList[i].mPort        = port;
			mSquareList[i].mCapacity    = capacity;
			mSquareList[i].mOnlineUsers = 0;
			mSquareList[i].mSession     = session;
			mSquareList[i].mStatus      = STATUS_SMOOTH;
			mSquareList[i].mType        = SQUARE_NORMAL;

			mSquareCount++;
			return i;
		}
	}
	return -1;
}

/* Removes the square at the specified index. */
bool SquareManager::Remove( uint32_t index )
{
	memset( &mSquareList[index], 0, sizeof( Square ) );
	mSquareCount--;

	return true;
}

/* Searches the squarelist for a square with the specified name. */
Square *SquareManager::Find( const char *name )
{
	for ( int i = 0; i < MAX_SQUARES; i++ )
		if ( mSquareList[i].mSession != NULL && ( _stricmp( mSquareList[i].mName, name ) == 0 ) )
			return &mSquareList[i];

	return NULL;
}

/* Initializes the square manager. */
void SquareManager::Initialize()
{
	/*memset( mSquareList, 0, sizeof( CSquare ) * MAX_SQUARES );*/
}