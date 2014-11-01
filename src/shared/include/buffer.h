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
#ifndef __SOLDIN_BUFFER_H__
#define __SOLDIN_BUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <shared.h>

#define BUFFER_OK    0
#define BUFFER_ERROR 1

class Buffer {
public:
	Buffer();
	Buffer( const char *data, size_t len );
	~Buffer();

	int            Resize( size_t size );
	void           Clear();
	int            Slice( char *dest, size_t len, size_t offset = 0 );
	inline void    Seek( size_t offset ) { mOffsetRead = MIN( offset, mOffsetWrite ); }
	inline char   *Content() const { return mBuffer; }
	inline size_t  Size() const { return mOffsetWrite; }

	int            Write( const char *data, size_t len );
	inline int     Write( const uint8_t *data, size_t len ) { return Write( (const char *)data, len ); }
	inline void    WriteFloat( float value ) { Write( (char *)&value, 4 ); }
	inline void    WriteByte( uint8_t value ) { Write( (char *)&value, 1 ); }
	inline void    WriteUInt16( uint16_t value ) { Write( (char *)&value, 2 ); }
	inline void    WriteUInt32( uint32_t value ) { Write( (char *)&value, 4 ); }
	inline void    WriteInt32( int value ) { return WriteUInt32( (uint32_t)value ); }
	inline void    WriteInt16( short value ) { return WriteUInt16( (uint16_t)value ); }
	void           WriteString( const char *str );
	void           WriteWideString( const wchar_t *str );

	int            Read( char *dest, size_t len );
	float          ReadFloat();
	uint8_t        ReadByte();
	uint16_t       ReadUInt16();
	uint32_t       ReadUInt32();
	inline int     ReadInt32() { return (int)ReadUInt32(); }
	inline short   ReadInt16() { return (short)ReadUInt16(); }
	size_t         ReadString( char *buffer, size_t size );
	const char    *ReadString();
	size_t         ReadWideString( wchar_t *buffer, size_t size );
	const wchar_t *ReadWideString();

	/* Gets the byte at the specified index in the buffer. */
	inline byte operator[] ( size_t index ) const
	{
		if ( index < 0 || index >= mBufferSize )
			return 0;

		return mBuffer[index];
	}

	/* Writes the content of the specified buffer to this buffer. */
	void Write( const Buffer &buffer )
	{
		if ( buffer.Size() > 0 )
			Write( buffer.Content(), buffer.Size() );
	}

private:
	char  *mBuffer;
	size_t mBufferSize;
	size_t mOffsetWrite;
	size_t mOffsetRead;
};

static char    __utf8_buff[1025];
static wchar_t __utf16_buff[1025];

/* Converts a specified UTF-16 string to UTF-8. */
inline static const char *UTF8(const wchar_t *instr)
{
	wcstombs(__utf8_buff, instr, 1024);
	return __utf8_buff;
}

/* Converts a specified UTF-8 string to UTF-16. */
inline static const wchar_t *UTF16(const char *instr)
{
	mbstowcs(__utf16_buff, instr, 1024);
	return __utf16_buff;
}

#endif /* __SOLDIN_BUFFER_H__ */
