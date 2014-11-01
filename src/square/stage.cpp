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
#include <stagemanager.h>
#include <stage.h>

/* Initializes a new stage. */
Stage::Stage(uint32_t stage_group, uint32_t level, uint32_t max_players): 
	mPlayerCount( 0 ), 
	mMaxPlayers( max_players ), 
	mStageGroup( stage_group ), 
	mNextObjectId( 10000 ), 
	mHub( false ), 
	mClosed( false ), 
	mLevel( level )
{
	mPlayers = (PlayerSession **)malloc( sizeof( PlayerSession * ) * max_players );
	if ( mPlayers == NULL )
	{
		// TODO: Error.
		return;
	}
	memset( mPlayers, 0, sizeof( PlayerSession * ) * max_players );
}

/* Destroy the stage. */
Stage::~Stage()
{
	if ( mPlayerCount > 0 )
	{
		// TODO: Move all players on this stage back back to the square stage.
	}

	if ( mPlayers != NULL ) free( mPlayers );
}

#define MSG_STAGE_CHANGE 0x7260

/* Adds the specified player to the stage. */
int Stage::Join( PlayerSession *player )
{
	if ( mPlayerCount == mMaxPlayers )
		return STERR_FULL;

	if ( mClosed ) /* Stage has been closed, nobody can join anymore. */
		return STERR_CLOSED;

	for ( uint32_t i = 0; i < mMaxPlayers; i++ )
	{
		if ( mPlayers[i] == NULL )
		{
			mPlayers[i] = player;
			Transfer( player );

			mPlayerCount++;
			return STERR_NONE;
		}
	}
	return STERR_UNKNOWN;
}

/* Removes the specified player from the stage. */
int Stage::Leave( PlayerSession *player )
{
	for ( uint32_t i = 0; i < mMaxPlayers; i++ )
	{
		if ( mPlayers[i] == NULL )
		{
			if ( i < ( mPlayerCount - 1 ) )
			{
				mPlayers[i] = mPlayers[mPlayerCount - 1];
				mPlayers[mPlayerCount - 1] = NULL;
			}
			else mPlayers[i] = NULL;

			mPlayerCount--;
			if ( mPlayerCount == 0 && !mHub )
			{
				StageManager::Destroy( mStageId );
			}

			return STERR_NONE;
		}
	}
	return STERR_PLNOTFOUND;
}

/* Sends a packet to all players on the stage. */
void Stage::Send( Buffer &packet, uint16_t cmd, int exclude_id )
{
	for ( uint32_t i = 0; i < mPlayerCount; i++ )
	{
		if ( mPlayers[i] == NULL || mPlayers[i]->mSessionId == exclude_id )
			continue;

		mPlayers[i]->Send( packet, cmd );
	}
}

/* Sends all stage information to the player. */
void Stage::Transfer( PlayerSession *player )
{

			/*Buffer unknownpkt;
			unknownpkt.WriteWideString(UTF16(player->GetCharacter()->name)); // Charactername.
			unknownpkt.WriteUInt32(0x529E424F);                              // Stagegroup Hash.
			unknownpkt.WriteUInt32(m_stage_group);                           // Stagegroup ID.
			unknownpkt.WriteUInt16(m_level);                                 // Stage Index.
			unknownpkt.WriteUInt32(0x393C107A);                              // Unknown list hash.
			unknownpkt.WriteUInt32(0);
			player->Send(unknownpkt, 0xE9FD);

			/* Square information. 
			Buffer stagepkt;
			stagepkt.WriteWideString(UTF16(player->GetCharacter()->name)); // Charactername.
			stagepkt.WriteUInt32(0x529E424F);                              // Stagegroup Hash.
			stagepkt.WriteUInt32(m_stage_group);                           // Stagegroup ID.
			stagepkt.WriteUInt16(m_level);                                 // Stage Index.
			stagepkt.WriteUInt32(0x393C107A);                            // Unknown list hash.
			stagepkt.WriteUInt32(0);
			player->Send(stagepkt, MSG_STAGE_CHANGE);*/

}
