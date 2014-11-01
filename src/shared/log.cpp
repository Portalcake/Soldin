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
#include <log.h>
#include <stdarg.h>
#include <time.h>
#include <console.h>

/* Initialize the default log files. */
#if defined( _GATEWAY )
Log ServerLog( "logs/gateway-server.log" );
Log ErrorLog ( "logs/gateway-error.log"  );
Log DebugLog ( "logs/gateway-debug.log"  );
#elif defined( _SQUARE )
Log ServerLog( "logs/square-server.log"  );
Log ErrorLog ( "logs/square-error.log"   );
Log DebugLog ( "logs/square-debug.log"   );
#endif

/* Initializes a new instance of the Log class. */
Log::Log( const char *file, bool verbose ): mVerbose( verbose )
{
	mFile = fopen( file, "a+" );
	if ( mFile == NULL )
	{
		printf( "Unable to open '%s' for logging...\n", file );
		return;
	}
}

/* Closes the log file. */
Log::~Log()
{
	if ( mFile != NULL ) fclose( mFile );
}

/* Writes a text message to the log. */
void Log::Write( const char *format, byte error_level, ... )
{
	va_list vl;
	va_start( vl, error_level );

	time_t t = time( NULL );
	struct tm *timeinfo = localtime( &t );

	char timestr[80];
	size_t len = strftime( timestr, 80, "%X", timeinfo );

	if ( mFile != NULL )
	{
		fprintf( mFile, "[%s] ", timestr );
		vfprintf( mFile, format, vl );
		fflush( mFile );
	}

	/* Output to console... */
	if ( mVerbose )
	{
		printf( "[%s] ", timestr );
		switch ( error_level ) 
		{
			case E_INFO:    Console::SetForeColor( FGC_GRAY );   break;
			case E_NOTICE:  Console::SetForeColor( FGC_WHITE );  break;
			case E_WARNING: Console::SetForeColor( FGC_YELLOW ); break;
			case E_ERROR:   Console::SetForeColor( FGC_RED );    break;
			case E_DEBUG:   Console::SetForeColor( FGC_BLUE );   break;
			case E_SUCCESS: Console::SetForeColor( FGC_GREEN );  break;
		}

		vprintf( format, vl );
		Console::SetForeColor( FGC_GRAY );
	}
	va_end( vl );
}
