#if !defined( PORTAL_UI_CONTROLLER_H )
#define PORTAL_UI_CONTROLLER_H

#if defined( WIN32 )
#pragma once
#endif

#include "igamesystem.h"

extern void ResetAllPlayersStats();

class CPortal_UI_Controller : public CAutoGameSystem
{
public:
	CPortal_UI_Controller();
	virtual ~CPortal_UI_Controller();

public:
	// CAutoGameSystem
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();

public:
	void OnLevelStart( float flDisplayTime = 0.f );
	void OnLevelEnd( float flDisplayTime = 0.f );
};

extern CPortal_UI_Controller g_portal_ui_controller;

#endif
