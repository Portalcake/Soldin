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
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log.h>

/* Removes the white space characters from the input string. */
size_t trim( const char *in, char *out )
{
	size_t len = strlen( in );
	if ( len > 0 )
	{
		size_t i = 0, j = 0;
		while ( in[i] <= 32 || in[i] > 128 )
		{
			if ( in[i] == 0 )
			{
				out[0] = 0;
				return 0;
			}
			else i++;
		}

		for ( ; i < len; i++, j++ )
			out[j] = in[i];

		for ( len = j; len >= 0; len-- )
		{
			if ( out[len] > 32 && out[len] <= 128 )
			{
				out[len + 1] = 0;
				break;
			}
		}
	}
	return len;
}

/* Initializes a new instance of the Config class. */
Settings::Settings( const char *path ): mCount(0), mList(NULL)
{
	FILE *fp = fopen( path, "rb" );
	if ( !fp ) 
	{
		ErrorLog.Write( "Unable to open configuration file '%s'.", E_ERROR, path );
		return;
	}

	/* Get the size of the file. */
	fseek( fp, 0, SEEK_END );
	long size = ftell( fp );
	if ( size == 0 ) 
	{
		fclose( fp );
		return;
	}
	fseek( fp, 0, SEEK_SET );


	char *buffer = (char *)malloc( size + 1 );
	fread( buffer, size, 1, fp );
	fclose( fp );
	buffer[size] = 0;

	size_t list_size = 100;
	mList = (Setting **)malloc( list_size * sizeof( Setting * ) );
	memset( mList, 0, list_size * sizeof( Setting * ) );

	/* Parse the configuration file. */
	char *line = strtok( buffer, "\n" );
	while ( line != NULL )
	{
		int len_line = strlen( line ), len_key = 0, len_value = 0;
		if ( len_line != 0 && line[0] != ';' )
		{
			/* Get the character that seperate they key and value. */
			char *value = strchr( line, '=' );
			if ( value != NULL )
			{
				/* Calculate the length and */
				len_key = value - line;
				*value = 0;
				++value;

				len_value = strlen(value);
				if (len_key > 0)
				{
					mList[mCount] = (Setting *)malloc( sizeof( Setting ) );
					mList[mCount]->mName = (char *)malloc( len_key + len_value + 2 );
					mList[mCount]->mValue = mList[mCount]->mName + len_key + 1;

					trim( line, mList[mCount]->mName );
					trim( value, mList[mCount]->mValue );
					++mCount;

					/* Increase the size of the vartable if the limit as been reached. */
					if ( mCount == list_size )
					{
						list_size += 100;
						mList = (Setting **)realloc( mList, list_size * sizeof( Setting * ) );

						memset( mList + ( ( list_size - 100 ) * sizeof( Setting * ) ), 0, ( 100 * sizeof( Setting * ) ) );
					}
				}
			}
		}
		line = strtok( NULL, "\n" );
	}
	if ( mCount != list_size ) 
		mList = ( Setting ** )realloc( mList, mCount * sizeof( Setting * ) );

	/* Cleanup... */
	free( buffer );
}

/* Frees all memory allocated by the configuration. */
Settings::~Settings()
{
	if ( mCount > 0 && mList )
	{
		for ( size_t i = 0; i < mCount; i++ )
			free( mList[i]->mName );

		free( mList );
	}
}

/* Gets the string value of the setting with the specified name. */
const char *Settings::GetString( const char *name, const char *default )
{
	for ( size_t i = 0; i < mCount; i++ )
		if ( _stricmp( mList[i]->mName, name ) == 0 )
			return mList[i]->mValue;

	return default;
}

/* Gets the integer value of the setting with the specified name. */
int Settings::GetInt( const char *name, int default )
{
	for ( size_t i = 0; i < mCount; i++ )
		if ( _stricmp( mList[i]->mName, name ) == 0 )
			return atoi( mList[i]->mValue );

	return default;
}

/* Gets the floating point value of the setting with the specified name. */
float Settings::GetFloat( const char *name, float default )
{
	for ( size_t i = 0; i < mCount; i++ )
		if ( _stricmp( mList[i]->mName, name ) == 0 )
			return (float)atof( mList[i]->mValue );

	return default;
}

/* Gets the boolean value of the setting with the specified name. */
bool Settings::GetBool( const char *name, bool default )
{
	for ( size_t i = 0; i < mCount; i++ )
	{
		if ( _stricmp( mList[i]->mName, name ) == 0 )
		{
			const char *v = mList[i]->mValue;
			if ( ( _stricmp( v, "true" ) == 0 ) || ( _stricmp( v, "yes" ) == 0 ) || ( _stricmp( v, "1" ) == 0 ) || ( _stricmp( v, "on" ) == 0 ) )
				return true;

			return false;
		}
	}
	return default;
}
