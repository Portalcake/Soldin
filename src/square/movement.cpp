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
#include <playersession.h>
#include <movement.h>

/* Direction angles. */
static Vector3 g_vecAngles[9] = {
	{ -0.707099974155426, 0, -0.707099974155426 },
	{  0,                 0, -1                 },
	{  0.707099974155426, 0, -0.707099974155426 },
	{ -1,                 0,  0                 },
	{ 0,                  0,  0                 },
	{  1,                 0,  0                 },
	{ -0.707099974155426, 0,  0.707099974155426 },
	{  0,                 0,  1                 },
	{  0.707099974155426, 0,  0.707099974155426 },
};

/* Starts movement in the specified direction. */
void PlayerSession::StartMovement( uint8_t dir, bool dash )
{
	if ( mMoving )
	{
		if ( mMoveDirection != dir ) /* Are we changing direction? */
		{
			CalculatePosition();
		}
	}
	else mMoving = true;

	mMoveStartTick = GetTick();
	mMoveDirection = dir;
	mDirection     = g_vecAngles[dir - 1];
	SetAction( ACT_RUN );
}

/* Calculates the position based on the time elapsed. */
void PlayerSession::CalculatePosition()
{
	uint32_t distance = ( GetTick() - mMoveStartTick ) / 30;

	printf("move_distance: %d\n", distance);

	if ( distance > 0 )
	{
		switch ( mMoveDirection )
		{
			case DIR_NORTHWEST: 
				mPosition.x -= ( distance / 1.5f ); 
				mPosition.z += ( distance / 1.5f ); 
				break;

			case DIR_NORTH: 
				mPosition.z += distance; 
				break;

			case DIR_NORTHEAST: 
				mPosition.x += ( distance / 1.5f ); 
				mPosition.z += ( distance / 1.5f ); 
				break;

			case DIR_WEST:
				mPosition.x -= distance;
				break;

			case DIR_EAST:
				mPosition.x += distance;
				break;

			case DIR_SOUTHWEST:
				mPosition.x -= ( distance / 1.5f );
				mPosition.z -= ( distance / 1.5f );
				break;

			case DIR_SOUTH:
				mPosition.z -= distance;
				break;

			case DIR_SOUTHEAST:
				mPosition.x += ( distance / 1.5f );
				mPosition.z -= ( distance / 1.5f );
				break;
		}
	}
}

/* Stops movement. */
void PlayerSession::StopMovement()
{
	mMoving = false;
	CalculatePosition();

	SetAction( ACT_IDLE );
}
