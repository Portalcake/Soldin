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

Stage   *StageManager::mStages[MAX_STAGES];
uint32_t StageManager::mStageCount = 0;

/* Initializes the stage manager. */
void StageManager::Initialize()
{
	memset( mStages, 0, sizeof( Stage * ) * MAX_STAGES );
}

/* Creates a new stage. */
Stage *StageManager::Create( uint32_t stage_group, uint32_t level, uint32_t max_players )
{
	for ( int i = 0; i < MAX_STAGES; i++ )
	{
		if ( mStages[i] == NULL )
		{
			mStages[i] = new Stage( stage_group, level, max_players );
			mStages[i]->mStageId = i;

			mStageCount++;
			return mStages[i];
		}
	}
	return NULL;
}

/* Destroys a stage. */
void StageManager::Destroy( uint32_t stage_id )
{
	if ( mStages[stage_id] == NULL )
		return;

	delete mStages[stage_id];
	mStages[stage_id] = NULL;

	mStageCount--;
}
