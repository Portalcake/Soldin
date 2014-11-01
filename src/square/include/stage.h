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
#ifndef __SOLDIN_STAGE_H__
#define __SOLDIN_STAGE_H__

#include <shared.h>
#include <playersession.h>
#include <buffer.h>
#include <vector>

/* Stage error codes. */
#define STERR_NONE       0
#define STERR_FULL       1
#define STERR_UNKNOWN    2
#define STERR_PLNOTFOUND 3
#define STERR_CLOSED     4

typedef struct object_t {
	uint32_t id;
} StageObject;

/* Represents a single stage. */
typedef std::vector<object_t *> objectlist_t;

class Stage {
public:
	Stage( uint32_t stage_group, uint32_t level, uint32_t max_players );
	~Stage();

	int      Join( PlayerSession *player );
	int      Leave( PlayerSession *player );
	void     Send( Buffer &packet, uint16_t cmd, int exclude_id = -1 );
	void     Transfer( PlayerSession *player );
	uint32_t GetGroupID()     const { return mStageGroup; }
	uint32_t GetPlayerCount() const { return mPlayerCount; }
	uint32_t MaxPlayers()     const { return mMaxPlayers; }

	int  mStageId;
	bool mHub;
	bool mClosed;

private:
	PlayerSession **mPlayers;
	uint32_t        mPlayerCount;
	uint32_t        mMaxPlayers;
	uint32_t        mStageGroup;
	uint32_t        mLevel;
	uint32_t        mNextObjectId;
	objectlist_t    mObjects;
};

#endif /* __SOLDIN_STAGE_H__ */
