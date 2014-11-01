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
#ifndef __SOLDIN_ACCOUNT_H__
#define __SOLDIN_ACCOUNT_H__

#include <shared.h>
#include <character.h>
#include <vector>

typedef std::vector<CharacterData *> CharacterList;

struct account_info_t {
	uint32_t      mId;
	char          mName[33];
	char          mPassword[33];
	uint32_t      mMaxChars;
	uint32_t      mGmLevel;
	uint8_t       mStatus;
	CharacterList mCharacters;
	uint32_t      mLicenseCount;
	uint32_t      mLicenses[32];
};

typedef struct account_info_t AccountInfo;

#endif /* __SOLDIN_ACCOUNT_H__ */
