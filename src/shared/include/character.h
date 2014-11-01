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
#ifndef __SOLDIN_CHARACTER_H__
#define __SOLDIN_CHARACTER_H__

#include <shared.h>
#include <equipment.h>
#include <vector>

#define MAX_CHARACTERS 32

#define BAG_SIZE       20
#define MAX_BAGS       7
#define MAX_BANK_BOXES 5


struct item_info_t 
{
	uint64_t mId;
	uint32_t mItemId;
	uint32_t mAmount;

};
typedef struct item_info_t ItemInfo;

struct bag_license_t 
{
	uint32_t mId;
	uint8_t  mIndex;
	uint8_t  mStatus;
	time_t   mExpires;
};
typedef struct bag_license_t BagLicense;


class CharacterData {
public:
	uint32_t      mId;
	char          mName[33];
	uint32_t      mClassId;
	time_t        mLastPlayed;
	uint16_t      mLevel;
	uint32_t      mExperience;
	uint16_t      mPvpLevel;
	uint32_t      mPvpExperience;
	uint16_t      mWarLevel;
	uint32_t      mWarExperience;
	uint16_t      mRebirthLevel;
	uint16_t      mRebirthCount;
	uint32_t      mEquipmentCount;
	EquipmentInfo mEquipment[32];

#	ifdef _SQUARE
	uint32_t mMoney;
	ItemInfo mBags[MAX_BAGS][BAG_SIZE];

	std::vector<BagLicense *> *mBagLicenses;

	struct {
		uint32_t mMoney;
		ItemInfo mBags[MAX_BANK_BOXES][BAG_SIZE];
.
	} mBank;
#	endif
};

#endif /* __SOLDIN_CHARACTER_H__ */
