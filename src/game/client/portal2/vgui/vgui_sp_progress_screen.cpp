#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Label.h"
#include "vgui_base_progress_screen.h"

#include "portal2/basemodui.h"

// gay
#include "tier0/memdbgon.h"

using namespace BaseModUI;

class CVGUI_SP_ProgressSignScreen : public CVGUI_Base_ProgressSignScreen
{
	DECLARE_CLASS( CVGUI_SP_ProgressSignScreen, CVGUI_Base_ProgressSignScreen );

public:
	CVGUI_SP_ProgressSignScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_SP_ProgressSignScreen();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnTick();
	virtual void Paint();

protected:
	void UpdateLevelData();
	void InitCommunityTagData();
	void UpdateCommunityLevelData();
	int GetInfoIconForTag( const char *pszTag );
	bool AddIconForTag( const char *pszTag, CUtlVector<int> &vecUsedIcons, CUtlVector<int> &vecFreeIcons, int nIconNum );

private:
	struct TagToIcon_t
	{
		TagToIcon_t()
		{
			m_pszTag = NULL;
			m_pszIcon = NULL;
		}

		TagToIcon_t( const char *pszTag, const char *pszIcon )
		{
			m_pszTag = pszTag;
			m_pszIcon = pszIcon;
		}

		const char *m_pszTag;
		const char *m_pszIcon;
	};

	vgui::Label *m_pLevelProgressLabel;
	CUtlVector<TagToIcon_t> m_TagToIconMap;
	vgui::Label *m_pLevelNameLabel;
	int m_nNumInfoIcons;
	bool m_bInitializedLevelLabel;
	char m_pszCurrentLevelName[128];
	int m_nCurrentLevel;
	int m_nTotalLevels;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_SP_ProgressSignScreen, "sp_progress_sign" );

CVGUI_SP_ProgressSignScreen::CVGUI_SP_ProgressSignScreen( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, "CVGUI_SP_ProgressSignScreen" ) // Line 79
{
	m_pLevelProgressLabel = NULL;
	m_TagToIconMap.Purge();
	m_pLevelNameLabel = NULL;
	m_bInitializedLevelLabel = false;
	m_nCurrentLevel = 0;
	m_nTotalLevels = 0;
	m_pszCurrentLevelName[0] = NULL;

	InitCommunityTagData();
}

CVGUI_SP_ProgressSignScreen::~CVGUI_SP_ProgressSignScreen() // Line 23
{
}

bool CVGUI_SP_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData ) // Line 102
{
	if( BaseClass::Init( pKeyValues, pInitData ) )
	{
		m_flGreyedIconAlpha = 112.f;
		m_nMaxBGAlpha = 100;

		return true;
	}

	return false;
}

void CVGUI_SP_ProgressSignScreen::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 120
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLevelProgressLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LevelProgressLabel" ) );
	if( m_pLevelProgressLabel )
		m_pLevelProgressLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	m_pLevelNameLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LevelNameLabel" ) );
	if( m_pLevelNameLabel )
		m_pLevelNameLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	m_bInitializedLevelLabel = true;

	UpdateLevelData();
}

void CVGUI_SP_ProgressSignScreen::UpdateLevelData() // Line 149
{
	const CBaseModPanel &BaseModPanel = CBaseModPanel::GetSingleton();
	bool bIsCommunityMap = BaseModPanel.GetCurrentCommunityMapID() || V_stristr( engine->GetLevelName(), "puzzlemaker" );

	if( m_bInitializedLevelLabel && ( bIsCommunityMap || !GameRules()->IsMultiplayer() ) )
	{
		const char *pszCurrentLevelName = engine->GetLevelNameShort();
		if( !V_stricmp( m_pszCurrentLevelName, pszCurrentLevelName ) )
		{
			return;
		}

		V_strcpy_safe( m_pszCurrentLevelName, pszCurrentLevelName );

		BaseClass::ResetLevelData();

		if( bIsCommunityMap )
		{
			UpdateCommunityLevelData();
		}
		else
		{
			if( m_pLevelNameLabel )
			{
				m_pLevelNameLabel->SetVisible( false );
			}

			KeyValues *pSPIcons = g_ProgressSignScreenKeys.GetSPIconKeyValues();
			if( !pSPIcons )
				return;

			KeyValues *pMaps = pSPIcons->FindKey( "Maps" );
			if( !pMaps )
				return;

			KeyValues *pMap = pMaps->FindKey( pszCurrentLevelName );
			if( !pMap )
				return;

			KeyValues *pLevelNumber = pMap->FindKey( "level_number" );
			if( pLevelNumber )
				m_nCurrentLevel = pLevelNumber->GetInt();

			KeyValues *pTotalLevels = pMap->FindKey( "total_levels" );
			if( pTotalLevels )
				m_nTotalLevels = pTotalLevels->GetInt();

			if( m_pLevelProgressLabel && m_bInitializedLevelLabel )
				m_pLevelProgressLabel->SetText( VarArgs( "%02d/%02d", m_nCurrentLevel, m_nTotalLevels ) );

			KeyValues *pStartupSequence = pMap->FindKey( "startup" );
			if( pStartupSequence )
			{
				m_nStartupSequenceIndex = g_ProgressSignScreenKeys.FindStartupSequenceIndex( pStartupSequence->GetString() );
			}
			else
			{
				m_nStartupSequenceIndex = g_ProgressSignScreenKeys.FindStartupSequenceIndex( DEFAULT_INFO_PANEL_STARTUP_SEQUENCE );
			}

			KeyValues *pDirt = pMap->FindKey( "dirt" );
			if( pDirt )
				m_nDirtOverlayID = clamp( pDirt->GetInt(), 0, NUM_DIRT_OVERLAYS - 1 );

			KeyValues *pIcons = pMap->FindKey( "icons" );
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
					m_bHighlightedIcons[nIconNum] = pIcon->GetInt() != 0;

					if( nIconNum++ == 10 )
					{
						DevWarning( "Too many lightboard icons defined for map %s\n", pszCurrentLevelName );
						break;
					}
				}
			}
		}
	}
}

void CVGUI_SP_ProgressSignScreen::Paint() // Line 290
{
	float flCurrentAlpha = BaseClass::UpdateStartupSequence();
	BaseClass::RenderBackground();
	BaseClass::RenderLevelNumber( m_nCurrentLevel, flCurrentAlpha );
	if( m_pLevelProgressLabel )
		m_pLevelProgressLabel->SetAlpha( flCurrentAlpha );
	BaseClass::RenderLevelProgress( m_nCurrentLevel, m_nTotalLevels, flCurrentAlpha );
	BaseClass::RenderIcons();
	if( m_nDirtOverlayID != -1 )
		BaseClass::RenderDirtOverlays();
	BaseClass::Paint();
}

void CVGUI_SP_ProgressSignScreen::OnTick() // Line 316
{
	UpdateLevelData();
	BaseClass::OnTick();
}

void CVGUI_SP_ProgressSignScreen::UpdateCommunityLevelData() // Line 328
{
	m_nStartupSequenceIndex = g_ProgressSignScreenKeys.FindStartupSequenceIndex( DEFAULT_INFO_PANEL_STARTUP_SEQUENCE );
	m_nDirtOverlayID = -1;

	// NOTE(SanyaSho): The community maps are not necessary for now because we don't have puzzlemaker support.
	// You still can play on community maps but without the info screen.
	// TODO:

	// This code is looking for something in GameUI... And sets the icons for the map. Also it can set the "Preview" tag for raw puzzlemaker map.

	// REMOVEME(SanyaSho): just to prevent crash if it appears...
	m_nCurrentLevel = 13;
	m_nTotalLevels = 37;

	if( m_pLevelProgressLabel && m_bInitializedLevelLabel )
		m_pLevelProgressLabel->SetText( VarArgs( "%02d/%02d", m_nCurrentLevel, m_nTotalLevels ) );
}

void CVGUI_SP_ProgressSignScreen::InitCommunityTagData() // Line 456
{
	// NOTE(SanyaSho): The community maps are not necessary for now because we don't have puzzlemaker support.
	// You still can play on community maps but without the info screen.
	// TODO:
	m_TagToIconMap.AddToTail( TagToIcon_t( "Goo",							"goop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Reflector Cube",				"laser_cube" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Turret",						"turret" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Faith Plate",					"plate_fling" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Light Bridge",					"bridges" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Tractor Beam",					"tbeams" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Bounce Gel",					"paint_bounce" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Speed Gel",						"paint_speed" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Laser Field",					"danger_field" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Weighted Cube Dropper",			"cube_drop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Companion Cube Dropper",		"cube_drop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Reflector Cube Dropper",		"cube_drop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Edgeless Safety Cube Dropper",	"cube_drop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Franken Cube Dropper",			"cube_drop" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Tractor Beam Polarity",			"tbeam_polarity" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Laser Catcher",					"laser_power" ) );
	m_TagToIconMap.AddToTail( TagToIcon_t( "Cube Button",					"cube_button" ) );
	
/*
  for ( i = 0; g_pszLightboardIcons[i]; ++i )
	;
  this->m_nNumInfoIcons = i;
*/
}

int CVGUI_SP_ProgressSignScreen::GetInfoIconForTag( const char *pszTag ) // Line 490
{
	// NOTE(SanyaSho): The community maps are not necessary for now because we don't have puzzlemaker support.
	// You still can play on community maps but without the info screen.
	// TODO:

	return -1;
}

bool CVGUI_SP_ProgressSignScreen::AddIconForTag( const char *pszTag, CUtlVector<int> &vecUsedIcons, CUtlVector<int> &vecFreeIcons, int nIconNum ) // Line 512
{
	// NOTE(SanyaSho): The community maps are not necessary for now because we don't have puzzlemaker support.
	// You still can play on community maps but without the info screen.
	// TODO:

	return false;
}