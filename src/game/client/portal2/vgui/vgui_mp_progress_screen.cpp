#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Label.h"
#include "vgui_base_progress_screen.h"
#include "portal_mp_gamerules.h"

struct CVGUI_MP_ProgressSignScreen : public CVGUI_Base_ProgressSignScreen
{
	DECLARE_CLASS( CVGUI_MP_ProgressSignScreen, CVGUI_Base_ProgressSignScreen );

public:
	CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_MP_ProgressSignScreen();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnTick();
	virtual void Paint();

protected:
	void UpdateLevelLabel();

private:
	int m_nDay;
	vgui::Label *m_pBranchNumLabel;
	vgui::Label *m_pLevelProgressNumLabel;
	bool m_bInitializedLevelLabel;
	int m_nCurrentLevel;
	int m_nTotalLevels;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_ProgressSignScreen, "mp_progress_sign" );

CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_MP_ProgressSignScreen" ) // Line 55
{
	m_nDay = 1;
	m_pBranchNumLabel = NULL;
	m_pLevelProgressNumLabel = NULL;
	m_bInitializedLevelLabel = false;
	m_nCurrentLevel = 0;
	m_nTotalLevels = 0;
}

CVGUI_MP_ProgressSignScreen::~CVGUI_MP_ProgressSignScreen() // Line 67
{
}

void CVGUI_MP_ProgressSignScreen::Paint() // Line 73
{
	float flCurrentAlpha = BaseClass::UpdateStartupSequence();
	BaseClass::RenderBackground();
	BaseClass::RenderLevelNumber( m_nCurrentLevel, flCurrentAlpha );
	BaseClass::RenderLevelProgress( m_nCurrentLevel, m_nTotalLevels, flCurrentAlpha );
	BaseClass::RenderIcons();
	if( m_nDirtOverlayID != -1 )
		BaseClass::RenderDirtOverlays();
	BaseClass::Paint();
}

void CVGUI_MP_ProgressSignScreen::UpdateLevelLabel() // Line 97
{
	if( !g_pGameRules->IsMultiplayer() || !PortalMPGameRules() || !PortalMPGameRules()->IsCoOp() || !m_bInitializedLevelLabel )
		return;

	int nCoopSelection = PortalMPGameRules()->GetCoopSection();
	if( nCoopSelection <= 0 )
		return;

	int nCurrentLevel = nCoopSelection; //TODO: ?????? nCoopSelection + nCoopSelection = goatse

	if( ( nCoopSelection != m_nDay ) || ( nCurrentLevel != m_nCurrentLevel ) )
	{
		m_nCurrentLevel = nCurrentLevel;
		m_nTotalLevels = PortalMPGameRules()->GetBranchTotalLevelCount();
		m_nDay = nCoopSelection;

		char szBranchNumber[128];
		V_sprintf_safe( szBranchNumber, "#P2COOP_TrackName_%d", nCoopSelection );

		if( m_pBranchNumLabel && m_bInitializedLevelLabel )
		{
			m_pBranchNumLabel->SetText( szBranchNumber );
		}

		char szProgressNumbers[128];
		V_sprintf_safe( szProgressNumbers, "%02d/%02d", m_nCurrentLevel, m_nTotalLevels );

		if( m_pLevelProgressNumLabel && m_bInitializedLevelLabel )
		{
			m_pLevelProgressNumLabel->SetText( szProgressNumbers );
		}

		KeyValues *pMPIcons = g_ProgressSignScreenKeys.GetMPIconKeyValues();
		if( !pMPIcons )
			return;

		KeyValues *pTracks = pMPIcons->FindKey( "Tracks" ); // Check for "Tracks"
		if( !pTracks )
			return;

		KeyValues *pMaps = pMPIcons->FindKey( "Maps" ); // Check for "Maps"
		if( !pMaps )
			return;

		// MAP SETTINGS START (same as in SP panel)
		KeyValues *pMap = pMaps->FindKey( engine->GetLevelNameShort() ); // Check for map settings
		if( !pMap )
			return;

		KeyValues *pStartupSequence = pMap->FindKey( "startup" ); // Find "startup" in map settings
		if( pStartupSequence )
			m_nStartupSequenceIndex = g_ProgressSignScreenKeys.FindStartupSequenceIndex( pStartupSequence->GetString() );
		else
			m_nStartupSequenceIndex = g_ProgressSignScreenKeys.FindStartupSequenceIndex( DEFAULT_INFO_PANEL_STARTUP_SEQUENCE );

		KeyValues *pDirt = pMap->FindKey( "dirt" ); // Find "dirt" in map settings
		if( pDirt )
			m_nDirtOverlayID = clamp( pDirt->GetInt(), 0, NUM_DIRT_OVERLAYS - 1 );
		// MAP SETTINGS END

		char szTrackName[32];
		V_sprintf_safe( szTrackName, "Track_%d", m_nDay );

		KeyValues *pIcons = pTracks->FindKey( szTrackName ); // Find all available icons for this track
		if( pIcons )
		{
			int nIconNum = 0;

			for( KeyValues *pIcon = pIcons->GetFirstSubKey(); pIcon != NULL; pIcon = pIcon->GetNextKey() )
			{
				int i = 0;
				for( i = 0; g_pszLightboardIcons[i]; i++ )
				{
					if( !Q_strcmp( pIcon->GetName(), g_pszLightboardIcons[i] ) )
						break;
				}

				m_nDisplayedIcons[nIconNum] = i;

				// Get the highlighted icons for this map in this track
				for( KeyValues *pHighlightedIcon = pMap->GetFirstSubKey(); pHighlightedIcon != NULL; pHighlightedIcon = pHighlightedIcon->GetNextKey() )
				{
					if( !Q_stricmp( pIcon->GetName(), pHighlightedIcon->GetName() ) && pHighlightedIcon->GetInt() > 0 )
						m_bHighlightedIcons[nIconNum] = true;
				}

				if( nIconNum++ == 10 )
				{
					Warning( "Too many icons defined in the coop_lightboard_icons file\n" );
					break;
				}
			}
		}
	}
}

void CVGUI_MP_ProgressSignScreen::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 212
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pBranchNumLabel = dynamic_cast<vgui::Label *>( FindChildByName( "BranchNumLabel" ) );
	if( m_pBranchNumLabel )
		m_pBranchNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	m_pLevelProgressNumLabel = dynamic_cast<vgui::Label *>( FindChildByName( "LevelProgressNumLabel" ) );
	if( m_pLevelProgressNumLabel )
		m_pLevelProgressNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	m_bInitializedLevelLabel = true;

	UpdateLevelLabel(); // gap0 + 952
}

bool CVGUI_MP_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData ) // Line 239
{
	return BaseClass::Init( pKeyValues, pInitData );
}

void CVGUI_MP_ProgressSignScreen::OnTick() // Line 250
{
	UpdateLevelLabel(); // gap0 + 952
	BaseClass::OnTick();
}
