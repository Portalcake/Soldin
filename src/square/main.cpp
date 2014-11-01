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
#include <vector>
#include <buffer.h>
#include <log.h>
#include <console.h>
#include <settings.h>
#include <gatewayclient.h>
#include <sessionmanager.h>
#include <playersession.h>
#include <vector>
#include <database.h>
#include <crypt.h>
#include <time.h>
#include <square.h>
#include <stagemanager.h>

Socket         g_square_socket;
Settings       g_square_config("config/soldin_square.cfg");
GatewayClient *g_gateway;

uint16_t    cfg_square_port;
uint32_t    cfg_square_host;
uint32_t    cfg_square_capacity;
const char *cfg_square_name;
uint16_t    cfg_gateway_port;
const char *cfg_gateway_host;
const char *cfg_sql_host;
const char *cfg_sql_username;
const char *cfg_sql_password;
const char *cfg_sql_database;
uint16_t    cfg_sql_port;

std::vector<PlayerSession *> g_clients;

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
	printf("\n   Square Server v0.1 by Seipheroth...\n\n\n");

	Console::SetForeColor(FGC_GRAY);
	Console::SetTitle("Soldin Square Server");
}

void Accept()
{
	Socket *sock = g_square_socket.Accept();
	if ( sock != NULL )
	{
		PlayerSession *pl = new PlayerSession( sock );

		SessionManager::Create( SESS_USER, pl );
		if ( pl->mSessionId == INVALID_SESSION )
		{
			ServerLog.Write( "Connection rejected, maximum number of sessions reached.\n", E_NOTICE );
			delete pl;
		}
		else
		{
			ServerLog.Write( "[%d][CLIENT] %s connected.\n", E_INFO, pl->mSessionId, pl->GetSocket()->Address() );
			g_clients.push_back( pl );
		}
	}
}

void Update()
{
	for ( std::vector<PlayerSession *>::iterator i = g_clients.begin(); i != g_clients.end(); )
	{
		PlayerSession *pl = *i;
		pl->Update();

		if ( !pl->Connected() || pl->mEOF )
		{
			SessionManager::Destroy( pl->mSessionId );
			ServerLog.Write( "[%d][CLIENT] %s disconnected.\n", E_INFO, pl->mSessionId, pl->GetSocket()->Address() );
			
			delete pl;
			i = g_clients.erase(i);
		}
		else ++i;
	}
}

typedef struct npc_t {
	uint32_t id;
	Vector3 position;
	Vector3 direction;
} NPC;

void LoadNPCs()
{
	FILE *fp = fopen("data/npcs.txt", "rb");
	if (fp == NULL) 
	{
		return;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *file_data = (char *)malloc(size + 1);
	fread(file_data, size, 1, fp);
	fclose(fp);
	file_data[size] = 0;


	char *line = strtok(file_data, "\n");
	while (line != NULL)
	{
		if (line[0] != ';' && line[0] != '#' && strlen(line) > 0)
		{
			npc_t npc_info;
			int res = sscanf(line, "%u,%f,%f,%f,%f,%f,%f\n", &npc_info.id,
				&npc_info.position.x, &npc_info.position.y, &npc_info.position.z, 
				&npc_info.direction.x, &npc_info.direction.y, &npc_info.direction.z);

			if (res != -1)
				printf("NPC id=%u, pos={%.2f,%.2f,%.2f}\n", 
					npc_info.id, npc_info.position.x, npc_info.position.y, npc_info.position.z);

		}
		line = strtok(NULL, "\n");
	}
}

/* Main entry point of the application. */
int main()
{
	PrintBanner();

	LoadNPCs();

	srand((unsigned int)time(NULL));

	/* Initialize managers. */
	StageManager::Initialize();

	Square::Initialize();


	/* Load the square configuration. */
	cfg_square_name  = g_square_config.GetString("square_name", "Square");
	cfg_square_capacity = g_square_config.GetInt("square_capacity", 100);
	cfg_square_host = inet_addr(g_square_config.GetString("square_host", "127.0.0.1"));
	cfg_square_port = g_square_config.GetInt("square_port", 15551);
	ServerLog.Write("Square: %s (port: %d, capacity: %d)\n", E_NOTICE, cfg_square_name, cfg_square_port, cfg_square_capacity);

	/* Load the SQL configuration. */
	cfg_sql_host     = g_square_config.GetString("sql_host",     "localhost");
	cfg_sql_username = g_square_config.GetString("sql_username", "root");
	cfg_sql_password = g_square_config.GetString("sql_password", "");
	cfg_sql_database = g_square_config.GetString("sql_database", "soldin");
	cfg_sql_port     = g_square_config.GetInt(   "sql_port",     3306);

	if ( !DB::Connect( cfg_sql_host, cfg_sql_username, cfg_sql_password, cfg_sql_database, cfg_sql_port ) )
	{
		DB::LogError();
		exit( -1 );
	}
	else ServerLog.Write( "Connection with the MySQL server established.\n", E_SUCCESS );


	if (g_square_socket.Listen(cfg_square_port) != 0)
	{
		ErrorLog.Write("Failed to listen on port %d.", E_ERROR, cfg_square_port);
		exit(1);
	}
	ServerLog.Write("Accepting client connections on port %d.\n", E_INFO, cfg_square_port);


	/* Establish connection with the gateway server. */
	cfg_gateway_port = g_square_config.GetInt("gateway_port", 14440);
	cfg_gateway_host = g_square_config.GetString("gateway_host", "127.0.0.1");
	g_gateway = new GatewayClient( cfg_gateway_host, cfg_gateway_port );

	
	/* Main server loop. */
	while (true)
	{
		g_gateway->Update();

		Accept();

		Update();
		
		sleep(10);
	}
}