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
#ifndef __SOLDIN_SQUAREMANAGER_H__
#define __SOLDIN_SQUAREMANAGER_H__

#include <shared.h>
#include <socket.h>
#include <buffer.h>
#include <squaresession.h>

#define MAX_SQUARES 25
#define MAX_SQUARENAME_LEN 50
#define INVALID_SQUARE -1

enum squarestatus_t : uint32_t
{
	STATUS_SMOOTH    = 1,
	STATUS_AVERAGE   = 2,
	STATUS_BUSY      = 3,
	STATUS_FULL      = 4
};

enum squaretype_t : uint32_t
{
	SQUARE_NORMAL    = 0,
	SQUARE_SLIMERACE = 1,
	SQUARE_UNKNOWN   = 2,
	SQUARE_BEGINNER  = 3,
};

/* Represents a single square. */
class Square
{
public:
	char            mName[MAX_SQUARENAME_LEN];
	in_addr         mHostAddr;
	uint16_t        mPort;
	uint32_t        mCapacity;
	uint32_t        mOnlineUsers;
	squarestatus_t  mStatus;
	squaretype_t    mType;
	SquareSession  *mSession;

	inline bool IsActive() const { return (mSession != NULL); }
};

/* Represents the square list. */
class SquareManager
{
public:
	static int     Add( const char *name, uint32_t hostaddr, uint16_t port, uint32_t capacity, SquareSession *session );
	static bool	   Remove( uint32_t index );
	static Square *Find( const char *name );
	static void    Initialize();
	inline static  Square *GetList() { return mSquareList; }
	inline static  uint32_t GetSquareCount() { return mSquareCount; }

	/* Gets a pointer to the square at the specified index. */
	static Square *At( uint32_t index )
	{
		if ( index >= MAX_SQUARES )
			return NULL;

		return &mSquareList[index];
	}

private:
	static Square   mSquareList[MAX_SQUARES];
	static uint32_t mSquareCount;
};

#endif /* __SOLDIN_SQUAREMANAGER_H__ */
