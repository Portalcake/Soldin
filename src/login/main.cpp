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
#include <stdio.h>
#include <socket.h>
#include <playersession.h>
#include <vector>
#include <buffer.h>
#include <log.h>
#include <console.h>
#include <squaremanager.h>
#include <squaresession.h>
#include <settings.h>
#include <sessionmanager.h>
#include <database.h>

#define SOLDIN_VER "0.3"

using namespace std;

/* Globals */
Settings                 Config("config/soldin_gateway.cfg");

Socket                  g_gate_socket;
Socket                  g_square_socket;
vector<SquareSession *> g_squares;
vector<PlayerSession *> g_clients;

/* Global Configuration */
uint16_t                 cfg_gateway_port;
const char              *cfg_gateway_ip;
uint16_t                 cfg_square_port;
const char              *cfg_sql_host;
const char              *cfg_sql_username;
const char              *cfg_sql_password;
const char              *cfg_sql_database;
uint16_t                 cfg_sql_port;

/* Displays the server banner in the console. */
void PrintBanner()
{
	Console::SetForeColor(FGC_BLUE);
	printf("   _________      .__       .___.__\n");
	printf("  /   _____/ ____ |  |    __| _/|__| ____\n");
	printf("  \\_____  \\ /  _ \\|  |   / __ | |  |/    \\\n");
	printf("  /        (  <_> )  |__/ /_/ | |  |   |  \\\n");
	printf(" /_______  /\\____/|____/\\____ | |__|___|  /\n");
	printf("   Lunia \\/ Server Emulator  \\/         \\/\n");
	Console::SetForeColor(FGC_DARKBLUE);
	printf("\n   Gateway Server v%s by Seipheroth...\n\n\n", SOLDIN_VER);
	Console::SetForeColor(FGC_GRAY);
	Console::SetTitle("Soldin Gateway Server");
}

/* Accept incoming connection requests. */
void AcceptIncoming()
{
	/* Accept connections from game clients. */
	Socket *sock = g_gate_socket.Accept();
	if ( sock != NULL )
	{
		PlayerSession *cl = new PlayerSession( sock );

		SessionManager::Create( SESS_USER, cl );
		if ( cl->mSessionId == INVALID_SESSION )
		{
			ServerLog.Write( "Connection rejected, maximum number of sessions reached.\n", E_WARNING );
			delete cl;
		}
		else 
		{
			ServerLog.Write( "[%d][CLIENT] %s connected.\n", E_INFO, cl->mSessionId, cl->GetSocket()->Address() );
			g_clients.push_back( cl );
		}
	}

	/* Accept connections from square servers. */
	sock = g_square_socket.Accept();
	if ( sock != NULL )
	{
		SquareSession *square = new SquareSession( sock );

		SessionManager::Create( SESS_SQUARE, square );
		if ( square->mSessionId == INVALID_SESSION )
		{
			ServerLog.Write( "Connection rejected, maximum number of sessions reached.\n", E_NOTICE );
			delete square;
		}
		else
		{
			ServerLog.Write( "[%d][SQUARE] %s connected.\n", E_INFO, square->mSessionId, square->GetSocket()->Address() );
			g_squares.push_back(square);
		}
	}
}

/* Iterates through all connected clients and processes updates. */
void Update()
{
	PlayerSession *cl;
	for ( vector<PlayerSession *>::iterator i = g_clients.begin(); i != g_clients.end(); )
	{
		cl = *i;
		cl->Update();

		if ( !cl->IsConnected() || cl->mEOF )
		{
			if ( !SessionManager::Destroy( cl->mSessionId ) )
			{
				ErrorLog.Write( "Failed to remove session %d.\n", E_ERROR, cl->mSessionId );
			}

			ServerLog.Write( "[%d][CLIENT] %s disconnected.\n", E_INFO, cl->mSessionId, cl->GetSocket()->Address() );
			i = g_clients.erase(i);

			delete cl;
		}
		else ++i;
	}

	SquareSession *square;
	for ( vector<SquareSession *>::iterator i = g_squares.begin(); i != g_squares.end(); )
	{
		square = *i;
		square->Update();

		if ( !square->IsConnected() || square->mEOF )
		{
			if ( !SessionManager::Destroy( square->mSessionId ) )
			{
				ErrorLog.Write( "Failed to remove session %d.\n", E_ERROR, square->mSessionId );
			}

			ServerLog.Write( "[%d][SQUARE] %s disconnected.\n", E_INFO, square->mSessionId, square->GetSocket()->Address() );
			i = g_squares.erase(i);

			delete square;
		}
		else ++i;
	}
}

/* Main entry point of the application. */
int main()
{
	PrintBanner();

	/* Load the configuration. */
	cfg_gateway_port = Config.GetInt( "gateway_port", 15550 );
	cfg_gateway_ip   = Config.GetString( "gateway_ip", "127.0.0.1" );
	cfg_square_port  = Config.GetInt( "square_port",  14440 );

	/* Start listening for connections from clients. */
	if ( g_gate_socket.Listen( cfg_gateway_port ) != 0 )
	{
		ErrorLog.Write( "Failed to listen on port %d.", E_ERROR, cfg_gateway_port );
		exit( -1 );
	}
	ServerLog.Write( "Server started on port %d.\n", E_SUCCESS, cfg_gateway_port );

	/* Start listening for square's. */
	if ( g_square_socket.Listen( cfg_square_port ) != 0 )
	{
		ErrorLog.Write( "Failed to listen on port %d.", E_ERROR, cfg_square_port );
		exit( -2 );
	}
	ServerLog.Write( "Accepting square connections on port %d.\n", E_SUCCESS, cfg_square_port );

	/* Load the SQL configuration. */
	cfg_sql_host     = Config.GetString( "sql_host", "localhost" );
	cfg_sql_username = Config.GetString( "sql_username", "root" );
	cfg_sql_password = Config.GetString( "sql_password", "" );
	cfg_sql_database = Config.GetString( "sql_database", "soldin" );
	cfg_sql_port     = Config.GetInt( "sql_port", 3306);

	if ( !DB::Connect( cfg_sql_host, cfg_sql_username, cfg_sql_password, cfg_sql_database, cfg_sql_port ) )
	{
		DB::LogError();
		exit( -3 );
	}
	else ServerLog.Write("Connection with the MySQL server established.\n", E_SUCCESS);

	/* Main server loop. */
	while (true)
	{
		/* Accept incoming connection requests. */
		AcceptIncoming();

		/* Update all connected clients. */
		Update();
	}
	return 0;
}
