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
#include <buffer.h>

/* Initializes a new instance of the Buffer class. */
Buffer::Buffer(): mOffsetWrite( 0 ), mOffsetRead( 0 ), mBufferSize( 0 ), mBuffer( NULL ) { }

/* Initializes a new instance of the Buffer class. */
Buffer::Buffer( const char *data, size_t len ): mOffsetWrite( len ), mOffsetRead( 0 )
{
	mBuffer = (char *)malloc( len );
	memcpy( mBuffer, data, len );
}

/* Frees the allocated buffer. */
Buffer::~Buffer()
{
	if ( mBuffer != NULL ) free( mBuffer );
}

/* Writes the specified data to the buffer. */
int Buffer::Write( const char *data, size_t len )
{
	size_t req_size = mOffsetWrite + len;
	if ( mBuffer == NULL)
	{
		mBufferSize = req_size * 2;
		mBuffer = (char *)malloc( mBufferSize );
		if ( mBuffer == NULL )
			return BUFFER_ERROR;
		
	}
	else if ( req_size > mBufferSize )
	{
		mBufferSize = req_size * 2;
		mBuffer = (char *)realloc( mBuffer, mBufferSize );
		if ( mBuffer == NULL )
			return BUFFER_ERROR;
	}

	memcpy( mBuffer + mOffsetWrite, data, len );
	mOffsetWrite = req_size;
	return 0;
}

/* Reads data from the buffer and writes it to the specified desination. */
int Buffer::Read( char *dest, size_t len )
{
	if ( mBuffer == NULL || len == 0 )
		return BUFFER_ERROR;

	size_t req_size = mOffsetRead + len;
	if ( req_size <= mOffsetWrite )
	{
		memcpy( dest, mBuffer + mOffsetRead, len );
		mOffsetRead = req_size;

		return 0;
	}
	return BUFFER_ERROR;
}

/* Resizes the buffer to the specified size. */
int Buffer::Resize( size_t size )
{
	if ( size == mBufferSize )
		return 0;

	if ( size == 0 && mBuffer != NULL )
	{
		free( mBuffer );
		mBuffer = NULL;

		mBufferSize = mOffsetWrite = mOffsetRead = 0;
		return BUFFER_OK;
	}

	mBufferSize = size;
	if ( mBuffer == NULL )
	{
		mBuffer = (char *)malloc( mBufferSize );
		if ( mBuffer == NULL )
			return BUFFER_ERROR;

		mOffsetWrite = mOffsetRead = 0;
		return BUFFER_OK;
	}

	mBuffer = (char *)realloc( mBuffer, mBufferSize );
	if ( mBuffer == NULL )
		return BUFFER_ERROR;

	mOffsetWrite = MIN( size, mOffsetWrite );
	mOffsetRead = MIN( size, mOffsetRead );
	return BUFFER_OK;
}

/* Clears the buffer by releasing the allocated memory. */
void Buffer::Clear()
{
	mBufferSize = mOffsetWrite = mOffsetRead = 0;
	if ( mBuffer != NULL )
	{
		free( mBuffer );
		mBuffer = NULL;
	}
}

/* Copies a slice of data from the buffer to the specified destination and removes it from the buffer. */
int Buffer::Slice( char *dest, size_t len, size_t offset )
{
	if ( ( offset + len ) > mOffsetWrite || len > mOffsetWrite || len == 0)
		return BUFFER_ERROR;

	memcpy( dest, mBuffer + offset, len );

	/* Move data after the sliced data forward. */
	int bytes_left = mOffsetWrite - ( offset + len );
	if ( bytes_left > 0 )
		memmove( mBuffer + offset, mBuffer + offset + len, bytes_left );

	mOffsetRead = MIN( offset, mOffsetRead );
	mOffsetWrite -= len;
	return 0;
}

/* Reads a byte (unsigned 8-bit integer) from the buffer. */
byte Buffer::ReadByte()
{
	mOffsetRead++;

	return mBuffer[mOffsetRead - 1];
}

/* Reads a unsigned 16-bit integer from the buffer. */
uint16_t Buffer::ReadUInt16()
{
	uint16_t value = 0;
	memcpy( &value, mBuffer + mOffsetRead, sizeof( uint16_t ) );
	mOffsetRead += sizeof( uint16_t );

	return value;
}

/* Reads a unsigned 32-bit integer from the buffer. */
uint32_t Buffer::ReadUInt32()
{
	uint32_t value = 0;
	memcpy( &value, mBuffer + mOffsetRead, sizeof( uint32_t ) );
	mOffsetRead += sizeof( uint32_t );

	return value;
}

/* Reads a floating point integer from the buffer. */
float Buffer::ReadFloat()
{
	float value = 0.0f;
	memcpy( &value, mBuffer + mOffsetRead, sizeof( float ) );
	mOffsetRead += sizeof( float );

	return value;
}

/* Reads a string from the buffer and writes it to the specified destination. */
size_t Buffer::ReadString( char *dest, size_t size )
{
	uint16_t strlen = (uint16_t)MIN( ( mOffsetWrite - mOffsetRead ), MIN( size, *( ( uint16_t * )( mBuffer + mOffsetRead ) ) ) );
	if ( strlen > 0 )
	{
		memcpy( dest, mBuffer + mOffsetRead + 2, strlen );
	}

	mOffsetRead += ( strlen + 2 );
	return strlen;
}

/* Gets a pointer to a string within the buffer. */
const char *Buffer::ReadString()
{
	uint16_t strlen = (uint16_t)MIN( ( mOffsetWrite - mOffsetRead ), *( ( uint16_t * )( mBuffer + mOffsetRead ) ) );

	const char *p = (char *)( mBuffer + mOffsetRead + 2 );
	mOffsetRead += strlen + 2;

	return p;
}

/* Reads a string from the buffer and writes it to the specified destination. */
size_t Buffer::ReadWideString( wchar_t *dest, size_t size )
{
	uint16_t strlen = (uint16_t)MIN( ( mOffsetWrite - mOffsetRead ), MIN( size, *( ( uint16_t * )( mBuffer + mOffsetRead ) ) ) );

	if ( strlen > 0 )
	{
		memcpy( dest, mBuffer + mOffsetRead + 2, strlen * sizeof( wchar_t ) );
	}

	mOffsetRead += ( ( strlen * sizeof( wchar_t ) ) + 2 );
	return strlen;
}

/* Gets a pointer to a string within the buffer. */
const wchar_t *Buffer::ReadWideString()
{
	uint16_t strlen = (uint16_t)MIN( ( mOffsetWrite - mOffsetRead ), *( ( uint16_t * )( mBuffer + mOffsetRead ) ) );

	const wchar_t *p = (wchar_t *)( mBuffer + mOffsetRead + 2 );
	mOffsetRead += ( strlen * 2 ) + 2;

	return p;
}

/* Writes a character string to the buffer. */
void Buffer::WriteString( const char *str )
{
	uint16_t size = strlen( str ) + 1;

	Write( (char *)&size, 2 );
	Write( str, size );
}

/* Writes a wide character string to the buffer. */
void Buffer::WriteWideString( const wchar_t *str )
{
	uint16_t size = wcslen( str ) + 1;

	Write( (char *)&size, 2 );
	Write( (char *)str, size * sizeof( wchar_t ) );
}
