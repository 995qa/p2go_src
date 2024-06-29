//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== portal_client.cpp ========================================================

  Portal client/server game specific stuff

*/

#include "cbase.h"
#include "portal_player.h"
#include "portal_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "entitylist.h"
#include "physics.h"
#include "game.h"
#include "player_resource.h"
#include "engine/IEngineSound.h"
#include "player.h"
#include "paint/paint_database.h" // PORTAL2

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Host_Say( edict_t *pEdict, bool teamonly );

//extern CBaseEntity*	FindPickerEntityClass( /*CBasePlayer* pPlayer,*/ char* classname);
extern bool			g_fGameOver;



/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer( edict_t *pEdict, const char *playername ) // Line 48
{
	// Allocate a CBasePlayer for pev, and call spawn
	CPortal_Player *pPlayer = CPortal_Player::CreatePlayer( "player", pEdict );
	pPlayer->PlayerData()->netname = AllocPooledString( playername );
	pPlayer->SetPlayerName(playername);
}

void ClientActive( edict_t *pEdict, bool bLoadGame ) // Line 56
{
	CPortal_Player *pPlayer = dynamic_cast< CPortal_Player* >( CBaseEntity::Instance( pEdict ) );
	Assert( pPlayer );

	pPlayer->InitialSpawn();

	if ( !bLoadGame )
	{
		pPlayer->Spawn();
	}
}



void ClientFullyConnect( edict_t *pEntity ) // Line 71
{
	CPortal_Player *pPlayer = dynamic_cast<CPortal_Player *>( CBaseEntity::Instance( pEntity ) );
	Assert( pPlayer );

	if( pPlayer && pPlayer->IsPlayer() ) // k... ???
	{
		PaintDatabase.SendPaintDataTo( pPlayer );
		pPlayer->OnFullyConnected();
	}
}

/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription() // Line 89
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Half-Life 2";
}



//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that 
//			classname that the player is nearest facing
//-----------------------------------------------------------------------------
CBaseEntity* FindEntity(edict_t* pEdict, char* classname) // Line 104
{
	// If no name was given set bits based on the picked
	if (FStrEq(classname, ""))
	{
		CBasePlayer* pPlayer = static_cast<CBasePlayer*>(GetContainingEntity(pEdict));
		if (pPlayer)
		{
			return pPlayer->FindPickerEntityClass(classname);
		}
	}
	return NULL;
}






//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void ClientGamePrecache( void ) // Line 126
{
}


// called by ClientKill and DeadThink
void respawn( CBaseEntity *pEdict, bool fCopyCorpse ) // Line 132
{
	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			((CPortal_Player *)pEdict)->CreateCorpse();
		}

		// respawn player
		pEdict->Spawn();
	}
	else
	{       // restart the entire server
		engine->ServerCommand("reload\n");
	}
}

void GameStartFrame( void ) // Line 151
{
	VPROF("GameStartFrame()");
	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = (teamplay.GetInt() != 0);
}




//=========================================================
// instantiate the proper game rules object
//=========================================================
void InstallGameRules() // Line 166
{
	if( IsGameRulesMultiplayer() )
		CreateGameRulesObject( "CPortalMPGameRules" );
	else
		CreateGameRulesObject( "CPortalGameRules" );
}
