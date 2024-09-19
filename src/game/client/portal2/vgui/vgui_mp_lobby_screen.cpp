#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Label.h"
#include "vgui_base_progress_screen.h"
#include "portal_mp_gamerules.h"
#include "c_portal_player.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"
#include "vgui_controls/ImagePanel.h"

// gay
#include "tier0/memdbgon.h"

class CObjectControlPanel : public CVGuiScreenPanel
{
	DECLARE_CLASS_SIMPLE( CObjectControlPanel, CVGuiScreenPanel );

public:
	CObjectControlPanel( vgui::Panel *parent, const char *panelName );
	virtual ~CObjectControlPanel();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual CBaseEntity *GetOwningObject();
	virtual void OnTickActive( CBaseEntity *pObj, CPortal_Player *pLocalPlayer );
	virtual vgui::Panel *TickCurrentPanel();
	virtual void SendToServerObject( const char *pMsg );
	virtual void OnTick();
	virtual void OnCommand( const char *command );

private:
	vgui::EditablePanel *m_pActivePanel;
	vgui::Panel *m_pCurrentPanel;
};

DECLARE_VGUI_SCREEN_FACTORY( CObjectControlPanel, "object_control_panel" );

CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName ) // Line 98
{
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( true );

	m_pActivePanel = new vgui::EditablePanel( this, "ActivePanel" );
	m_pActivePanel->SetPaintBackgroundEnabled( false );

	SetCursor( vgui::dc_arrow );
	m_pActivePanel->SetZPos( -1 );
}

CObjectControlPanel::~CObjectControlPanel() // Line 33
{
}

bool CObjectControlPanel::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData ) // Line 118
{
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	if( BaseClass::Init( pKeyValues, pInitData ) )
	{
		int x, y, w, h;
		GetBounds( x, y, w, h );
		m_pActivePanel->SetBounds( x, y, w, h );
		/*
     (*(void (__cdecl **)(vgui::EditablePanel *, _DWORD))(*(_DWORD *)this->m_pActivePanel + 136))(
	  this->m_pActivePanel,
	  0);
		*/ // SetVisible( false )?

		m_pCurrentPanel = m_pActivePanel;

		return true;
	}

	return false;
}

CBaseEntity *CObjectControlPanel::GetOwningObject() // Line 144
{
	return GetEntity(); // ???
}

void CObjectControlPanel::OnTickActive( CBaseEntity *pObj, CPortal_Player *pLocalPlayer ) // Line 161
{
}

vgui::Panel *CObjectControlPanel::TickCurrentPanel() // Line 166
{
	HACK_GETLOCALPLAYER_GUARD( __FUNCTION__ );

	m_pCurrentPanel->SetCursor( vgui::dc_arrow ); // ?????

	OnTickActive( GetOwningObject(), CPortal_Player::GetLocalPortalPlayer() );

	return m_pCurrentPanel;
}

void CObjectControlPanel::SendToServerObject( const char *pMsg ) // Line 182
{
	if( !GetOwningObject() )
		return;

	// TODO: seems to be unused...
}

void CObjectControlPanel::OnTick() // Line 197
{
	BaseClass::OnTick();

	if( GetOwningObject() )
	{
		// ?????
	}
}

void CObjectControlPanel::OnCommand( const char *command ) // Line 216
{
	BaseClass::OnCommand( command );
}



// STEPS

class CVGUI_MP_LobbyScreen: public CObjectControlPanel
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen, CObjectControlPanel );

public:
	CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnTick();

protected:
	virtual void UpdateData();

private:
	vgui::Label *m_pStepsB;
	vgui::Label *m_pStepsO;
	bool bIsAlreadyVisible;
	int m_nStepsB;
	int m_nStepsO;
	bool m_bAppliedSchemeSettings;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen, "mp_lobby_screen1" );

CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen" ) // Line 254
{
	SetScheme( "basemodui_scheme" );
	m_nStepsB = 0;
	m_nStepsO = 0;
	m_bAppliedSchemeSettings = false;
}

CVGUI_MP_LobbyScreen::~CVGUI_MP_LobbyScreen() // Line 233
{
}

void CVGUI_MP_LobbyScreen::UpdateData() // Line 267
{
	if( !g_pGameRules->IsMultiplayer() )
		return;

	// COOPTODO: CPortalMPStats
	m_nStepsB = 1337;
	m_nStepsO = 1337;

	m_nStepsB = clamp( m_nStepsB, 0, 99999999 ); // SanyaSho: replaced if > check with clamp
	m_nStepsO = clamp( m_nStepsO, 0, 99999999 ); // SanyaSho: replaced if > check with clamp

	if( m_pStepsB )
	{
		m_pStepsB->SetText( VarArgs( "%d", m_nStepsB ) );
	}

	if( m_pStepsO )
	{
		m_pStepsO->SetText( VarArgs( "%d", m_nStepsO ) );
	}
}

void CVGUI_MP_LobbyScreen::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 323
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pStepsB = dynamic_cast<vgui::Label *>( FindChildByName( "StepsB" ) );
	m_pStepsO = dynamic_cast<vgui::Label *>( FindChildByName( "StepsO" ) );

	m_bAppliedSchemeSettings = true;
}

bool CVGUI_MP_LobbyScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData ) // Line 337
{
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	return BaseClass::Init( pKeyValues, pInitData );
}

void CVGUI_MP_LobbyScreen::OnTick() // Line 351
{
	BaseClass::OnTick();

	if( !bIsAlreadyVisible )
	{
		SetVisible( true );
		bIsAlreadyVisible = true;
	}

	if( m_bAppliedSchemeSettings )
	{
		UpdateData();
	}
}


// PORTAL COUNT

class CVGUI_MP_LobbyScreen2 : public CVGUI_MP_LobbyScreen
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen2, CVGUI_MP_LobbyScreen );

public:
	CVGUI_MP_LobbyScreen2( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen2();

	virtual void UpdateData();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::Label *m_pNumPortals;
	int m_nPortals;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen2, "mp_lobby_screen2" );

CVGUI_MP_LobbyScreen2::CVGUI_MP_LobbyScreen2( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen2" ) // Line 391
{
	m_nPortals = 0;
}

CVGUI_MP_LobbyScreen2::~CVGUI_MP_LobbyScreen2() // Line 368
{
}

void CVGUI_MP_LobbyScreen2::UpdateData() // Line 401
{
	if( !g_pGameRules->IsMultiplayer() )
		return;

	// COOPTODO: CPortalMPStats
	m_nPortals = 1337 + 1337; // Blue + Orange portals

	m_nPortals = clamp( m_nPortals, 0, 9999 ); // SanyaSho: replaced if > check with clamp

	if( m_pNumPortals )
	{
		m_pNumPortals->SetText( VarArgs( "%d", m_nPortals ) );
	}
}

void CVGUI_MP_LobbyScreen2::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 454
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pNumPortals = dynamic_cast<vgui::Label *>( FindChildByName( "NumPortals" ) );
}


// TRAVELED PORTALS

class CVGUI_MP_LobbyScreen3 : public CVGUI_MP_LobbyScreen
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen3, CVGUI_MP_LobbyScreen );

public:
	CVGUI_MP_LobbyScreen3( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen3(); // Line 465

	virtual void UpdateData();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::Label *m_pNumPortals;
	int m_nPortals;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen3, "mp_lobby_screen3" );

CVGUI_MP_LobbyScreen3::CVGUI_MP_LobbyScreen3( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen3" ) // Line 488
{
	m_nPortals = 0;
}

CVGUI_MP_LobbyScreen3::~CVGUI_MP_LobbyScreen3() // Line 465
{
}

void CVGUI_MP_LobbyScreen3::UpdateData() // Line 498
{
	if( !g_pGameRules->IsMultiplayer() )
		return;

	// COOPTODO: CPortalMPStats
	m_nPortals = 1337 + 1337; // Blue + Orange portals

	m_nPortals = clamp( m_nPortals, 0, 9999 ); // SanyaSho: replaced if > check with clamp

	if( m_pNumPortals )
	{
		m_pNumPortals->SetText( VarArgs( "%d", m_nPortals ) );
	}
}

void CVGUI_MP_LobbyScreen3::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 551
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pNumPortals = dynamic_cast<vgui::Label *>( FindChildByName( "NumPortals" ) );
}


// HUGS

class CVGUI_MP_LobbyScreen4 : public CVGUI_MP_LobbyScreen
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen4, CVGUI_MP_LobbyScreen );

public:
	CVGUI_MP_LobbyScreen4( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen4(); // Line 561

	virtual void UpdateData();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::Label *m_pNumHugs;
	int m_nHugs;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen4, "mp_lobby_screen4" );

CVGUI_MP_LobbyScreen4::CVGUI_MP_LobbyScreen4( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen4" ) // Line 584
{
	m_nHugs = 0;
}

CVGUI_MP_LobbyScreen4::~CVGUI_MP_LobbyScreen4() // Line 561
{
}

void CVGUI_MP_LobbyScreen4::UpdateData() // Line 594
{
	if( !g_pGameRules->IsMultiplayer() )
		return;

	// COOPTODO: CPortalMPStats
	m_nHugs = 1337 + 1337; // Blue + Orange hugs

	m_nHugs = clamp( m_nHugs, 0, 999 ); // SanyaSho: replaced if > check with clamp

	if( m_pNumHugs )
	{
		m_pNumHugs->SetText( VarArgs( "%d", m_nHugs ) );
	}
}

void CVGUI_MP_LobbyScreen4::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 610
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pNumHugs = dynamic_cast<vgui::Label *>( FindChildByName( "NumHugs" ) );
}


// DISASTERS

class CVGUI_MP_LobbyScreen5 : public CVGUI_MP_LobbyScreen
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen5, CVGUI_MP_LobbyScreen );

public:
	CVGUI_MP_LobbyScreen5( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen5(); // Line 621

	virtual void UpdateData();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::Label *m_pDisasters;
	int m_nDisasters;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen5, "mp_lobby_screen5" );

CVGUI_MP_LobbyScreen5::CVGUI_MP_LobbyScreen5( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen5" ) // Line 644
{
	m_nDisasters = 0;
}

CVGUI_MP_LobbyScreen5::~CVGUI_MP_LobbyScreen5() // Line 621
{
}

void CVGUI_MP_LobbyScreen5::UpdateData() // Line 654
{
	if( !PortalMPGameRules() || !PortalMPGameRules()->IsMultiplayer() || !PortalMPGameRules()->IsCoOp() )
		return;

	if( m_nDisasters == 0 ) // IDA says !m_nDisasters, Ghidra says m_nDisasters == 0
	{
		int nActiveBranches = PortalMPGameRules()->GetActiveBranches();
		if( nActiveBranches <= 0 )
			return;

		int nBranch = 4;
		if( ( nActiveBranches - 1 ) <= 4 )
			nBranch = ( nActiveBranches - 1 );

		int nLevelCount = PortalMPGameRules()->GetBranchTotalLevelCount( nBranch );
		if( nLevelCount > 0 )
		{
			// TODO:
		}

		m_nDisasters = ( ( nBranch * 1.25f ) + 10.f ) + 8.f;
	}

	m_nDisasters = clamp( m_nDisasters, 0, 9999 ); // SanyaSho: replaced if > check with clamp

	if( m_pDisasters )
	{
		m_pDisasters->SetText( VarArgs( "%d", m_nDisasters ) );
	}
}

void CVGUI_MP_LobbyScreen5::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 689
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pDisasters = dynamic_cast<vgui::Label *>( FindChildByName( "Disasters" ) );
}


// SOCIAL CREDIT

class CVGUI_MP_LobbyScreen6 : public CVGUI_MP_LobbyScreen
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen6, CVGUI_MP_LobbyScreen );

public:
	CVGUI_MP_LobbyScreen6( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LobbyScreen6(); // Line 699

	virtual void UpdateData();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::Label *m_pSCPs;
	int m_nSCPs;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen6, "mp_lobby_screen6" );

CVGUI_MP_LobbyScreen6::CVGUI_MP_LobbyScreen6( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LobbyScreen6" ) // Line 722
{
	m_nSCPs = 0;
}

CVGUI_MP_LobbyScreen6::~CVGUI_MP_LobbyScreen6() // Line 699
{
}

void CVGUI_MP_LobbyScreen6::UpdateData() // Line 732
{
	if( !PortalMPGameRules() || !PortalMPGameRules()->IsMultiplayer() || !PortalMPGameRules()->IsCoOp() )
		return;

	if( m_nSCPs == 0 ) // Same as in CVGUI_MP_LobbyScreen5
	{
		int nActiveBranches = PortalMPGameRules()->GetActiveBranches();
		if( nActiveBranches <= 0 )
			return;

		// :thumbsup::skin-tone-1: Social credit +10
		m_nSCPs = 32 * nActiveBranches + RandomInt( -1000, 2000 );
	}

	m_nSCPs = clamp( m_nSCPs, 0, 99999 ); // SanyaSho: replaced if > check with clamp

	if( m_pSCPs )
	{
		m_pSCPs->SetText( VarArgs( "%d", m_nSCPs ) );
	}
}

void CVGUI_MP_LobbyScreen6::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 755
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pSCPs = dynamic_cast<vgui::Label *>( FindChildByName( "SCPs" ) );
}


///
/// LOBBY LEVEL SELECTION SCREENS
///

class CVGUI_MP_LevelSelectScreenBoxes : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
	//DECLARE_CLASS( CVGUI_MP_LevelSelectScreenBoxes, vgui::Panel );

public:
	CVGUI_MP_LevelSelectScreenBoxes( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LevelSelectScreenBoxes(); // Line 767

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void Paint();

	void SetDay( int nBranch, int nLevel, int nTotalLevels );

private:
	int m_nNumberTextureID;
	int m_nCurrentLevel;
};

CVGUI_MP_LevelSelectScreenBoxes::CVGUI_MP_LevelSelectScreenBoxes( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LevelSelectScreenBoxes" ) // Line 790
{
	SetScheme( "basemodui_scheme" );

	m_nCurrentLevel = 0;
}

CVGUI_MP_LevelSelectScreenBoxes::~CVGUI_MP_LevelSelectScreenBoxes() // Line 767
{
}

void CVGUI_MP_LevelSelectScreenBoxes::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 798
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_nNumberTextureID = vgui::surface()->DrawGetTextureId( "vgui/screens/vgui_coop_progress_board_numbers" );
	if( m_nNumberTextureID == -1 )
	{
		m_nNumberTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nNumberTextureID, "vgui/screens/vgui_coop_progress_board_numbers", 1, false );
	}

	SetPaintBackgroundEnabled( false );
}

void CVGUI_MP_LevelSelectScreenBoxes::PerformLayout() // Line 814
{
	BaseClass::PerformLayout();
}

void CVGUI_MP_LevelSelectScreenBoxes::Paint() // Line 820
{
	// TODO: THIS CODE IS ALMOST SAME AS IN VGUI_BASE_PROGRESS_SCREEN.CPP!

#if 0
	int w, h;
	GetSize( w, h );

	vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTexture( m_nNumberTextureID );

	float v2; // xmm0_4
	int v3; // edi
	int v4; // ebx
	int m_nCurrentLevel; // ecx
	float v6; // xmm1_4
	int v7; // esi
	int v8; // eax
	int v9; // ecx
	float v10; // xmm0_4
	int v11; // eax
	int v12; // eax
	int v13; // [esp+64h] [ebp-1Ch]
	float flNumX2; // [esp+68h] [ebp-18h]
	int nNumHeight; // [esp+78h] [ebp-8h]
	float flNumY2; // [esp+7Ch] [ebp-4h]

	v2 = (float)h * 0.49000001;
	v3 = (int)v2 / 3;
	nNumHeight = (int)v2;
	v4 = w - ( 2 * v3 + 38 );
	//g_pVGuiSurface->DrawSetColor( g_pVGuiSurface, 255, 255, 255, 255 );
	//g_pVGuiSurface->DrawSetTexture( g_pVGuiSurface, this->m_nNumberTextureID );
	m_nCurrentLevel = this->m_nCurrentLevel;
	v6 = 0.0;
	v7 = m_nCurrentLevel % 10;
	v8 = ( m_nCurrentLevel - m_nCurrentLevel % 10 ) / 10;
	v9 = 0;
	if( v8 > 5 )
	{
		v9 = 6;
		v6 = 0.5f;
	}
	v10 = (float)( v8 - v9 ) * 0.16666667;
	v11 = nNumHeight + 100;
	nNumHeight = v4 + v3;
	v13 = v11;
	g_pVGuiSurface->DrawTexturedSubRect(
	  v4,
	  100,
	  v4 + v3,
	  v11,
	  ( v10 ),
	  ( v6 ),
	  v10 + 0.16666667,
	  v6 + 0.5 );
	v12 = 0;
	flNumY2 = 0.0;
	if( v7 > 5 )
	{
		v12 = 6;
		flNumY2 = 0.5f;
	}
	flNumX2 = (float)( v7 - v12 ) * 0.16666667;

	g_pVGuiSurface->DrawSetColor( 255, 255, 255, 255 );
	g_pVGuiSurface->DrawSetTexture( m_nNumberTextureID );
	g_pVGuiSurface->DrawTexturedSubRect(
	  nNumHeight,
	  100,
	  v4 + 2 * v3,
	  v13,
	  ( flNumX2 ),
	  ( flNumY2 ),
	  flNumX2 + 0.16666667,
	  flNumY2 + 0.5 );
#endif

	BaseClass::Paint();
}

void CVGUI_MP_LevelSelectScreenBoxes::SetDay( int nBranch, int nLevel, int nTotalLevels ) // Line 874
{
	if( !PortalMPGameRules() || !PortalMPGameRules()->IsCoOp() )
		return;

	m_nCurrentLevel = nLevel;

	// ??????
	int nParentW, nParentH;
	GetSize( nParentW, nParentH );
}



#define MAX_LEVEL_ICON_PANELS 16

class CVGUI_MP_LevelSelectScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_MP_LevelSelectScreen, CVGuiScreenPanel );

public:
	CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_LevelSelectScreen();

private:
	int m_nDay;

	vgui::ImagePanel *m_pLevelIconPanelsB[MAX_LEVEL_ICON_PANELS];
	vgui::ImagePanel *m_pLevelIconPanelsBLock[MAX_LEVEL_ICON_PANELS];

	vgui::ImagePanel *m_pLevelIconPanelsO[MAX_LEVEL_ICON_PANELS];
	vgui::ImagePanel *m_pLevelIconPanelsOLock[MAX_LEVEL_ICON_PANELS];

	vgui::ImagePanel *m_pLevelIconPanelSelectionT;
	vgui::ImagePanel *m_pLevelIconPanelSelection;

	vgui::Label *m_pBranchTitleLabel;
	vgui::Label *m_pSelectLevelLabel;
	vgui::Label *m_pCourseNumberTitle;
	vgui::Label *m_pPercentCompletedTitle;

	vgui::ImagePanel *m_pLockIcon;
	CVGUI_MP_LevelSelectScreenBoxes *m_pBoxesPanel;

	bool bIsAlreadyVisible;
	bool m_bInitializedLevelLabel;

	int m_nBackgroundTextureID;
	int m_nNumberTextureID;
	bool m_bNeedsRefresh;
	int m_nCurrentLevel;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LevelSelectScreen, "mp_select_screen_branch_1" );

CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_LevelSelectScreen" )
{
	SetScheme( "basemodui_scheme" );

	m_nDay = 1;
	m_nCurrentLevel = 0;
	m_bInitializedLevelLabel = false;
	m_bNeedsRefresh = false;

	for( int i = 0; i < MAX_LEVEL_ICON_PANELS; i++ )
	{
		m_pLevelIconPanelsB[i] = new vgui::ImagePanel(this, VarArgs( "LevelIconB%d", i ) );
		m_pLevelIconPanelsBLock[i] = new vgui::ImagePanel( this, VarArgs( "LevelIconBLock%d", i ) );

		m_pLevelIconPanelsO[i] = new vgui::ImagePanel( this, VarArgs( "LevelIconO%d", i ) );
		m_pLevelIconPanelsOLock[i] = new vgui::ImagePanel( this, VarArgs( "LevelIconOLock%d", i ) );
	}

	m_pLevelIconPanelSelectionT = new vgui::ImagePanel( this, "IconPanelSelectionT" );
	m_pLevelIconPanelSelection = new vgui::ImagePanel( this, "IconPanelSelection" );

	m_pBoxesPanel = new CVGUI_MP_LevelSelectScreenBoxes( this, "LevelSelectScreenBoxes" );

	// Was uninitialized
	m_pBranchTitleLabel = NULL;
	m_pSelectLevelLabel = NULL;
	m_pCourseNumberTitle = NULL;
	m_pPercentCompletedTitle = NULL;
	bIsAlreadyVisible = false;
	m_nBackgroundTextureID = -1;
	m_nNumberTextureID = -1;
}

CVGUI_MP_LevelSelectScreen::~CVGUI_MP_LevelSelectScreen()
{
}
