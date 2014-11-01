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
#ifndef __SOLDIN_STAGEMANAGER_H__
#define __SOLDIN_STAGEMANAGER_H__

#include <shared.h>
#include <stage.h>

#define MAX_STAGES 1000

/* Represents the stage list.  */
class StageManager {
public:
	static void   Initialize();
	static Stage *Create( uint32_t stage_group, uint32_t level, uint32_t max_players );
	static void   Destroy( uint32_t stage_id );
	static Stage *At( uint32_t stage_id ) { return mStages[stage_id]; }

private:
	static Stage   *mStages[MAX_STAGES];
	static uint32_t mStageCount;
};

#endif /* __SOLDIN_STAGEMANAGER_H__ */
