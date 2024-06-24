#include "cbase.h"
#include "portal_gamestats.h"
#include "portal_player.h"
#include "portal2_research_data_tracker.h"
#include "portal_ui_controller.h"

// TODO(SanyaSho): this file is probably used for something else

CPortal_UI_Controller g_portal_ui_controller;

CPortal_UI_Controller::CPortal_UI_Controller() : BaseClass( "Portal_UI_Controller" ) // Line 367
{
}

CPortal_UI_Controller::~CPortal_UI_Controller() // Line 372
{
}

void ResetAllPlayersStats() // Line 378
{
	for( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CPortal_Player *pPlayer = ToPortalPlayer( UTIL_PlayerByIndex( i ) );
		if( pPlayer )
		{
			if( pPlayer->IsPlayer() )
				pPlayer->ResetThisLevelStats();
		}
	}
}

void CPortal_UI_Controller::LevelInitPostEntity() // Line 396
{
	ResetAllPlayersStats();
	if( !g_pPlayerPortalStatsController )
	{
		g_pPlayerPortalStatsController = dynamic_cast< CPortalStatsController *>( CreateEntityByName( "portal_stats_controller" ) );
		DispatchSpawn( g_pPlayerPortalStatsController, true );
	}
}

void CPortal_UI_Controller::LevelShutdownPreEntity() // Line 408
{
	if( g_pPlayerPortalStatsController )
	{
		UTIL_Remove( g_pPlayerPortalStatsController );
		g_pPlayerPortalStatsController = NULL;
	}
}

void CPortal_UI_Controller::OnLevelStart( float flDisplayTime ) // Line 418
{
	if( g_pPlayerPortalStatsController )
		g_pPlayerPortalStatsController->LevelStart( flDisplayTime );
}

void CPortal_UI_Controller::OnLevelEnd( float flDisplayTime ) // Line 425
{
	if( g_pPlayerPortalStatsController )
	{
		g_pPlayerPortalStatsController->LevelEnd( flDisplayTime );
		g_Portal2ResearchDataTracker.Event_LevelCompleted();
	}
}
