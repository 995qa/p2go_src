#include "cbase.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "mathlib/mathlib.h"
#include "coordsize.h"
#include "vgui_base_progress_screen.h"

// gay
#include "tier0/memdbgon.h"

CProgressSignScreenKeys::CProgressSignScreenKeys( char const *name ) : CAutoGameSystem( name ) // Line 55
{
	m_pPreloadedSPIconKeys = NULL;
	m_pPreloadedMPIconKeys = NULL;
	m_bStartupSequencesInitiailized = false;
	m_StartupSequences.Purge();
}

bool CProgressSignScreenKeys::Init() // Line 68
{
	if( !m_pPreloadedSPIconKeys )
	{
		KeyValues *pSP = new KeyValues( "sp_lightboard_icons" );
		m_pPreloadedSPIconKeys = pSP;
		pSP->LoadFromFile( filesystem, "scripts/vscripts/transitions/sp_lightboard_icons.txt" );
	}

	if( !m_pPreloadedMPIconKeys )
	{
		KeyValues *pMP = new KeyValues( "coop_lightboard_icons" );
		m_pPreloadedMPIconKeys = pMP;
		pMP->LoadFromFile( filesystem, "scripts/vscripts/coop/coop_lightboard_icons.txt" );
	}

	return SetupStartupSequenecs();
}

void SetSequenceFieldFromKey( KeyValues *pSequenceKey, const char *pszFieldName, float *flField ) // Line 97
{
	KeyValues *pData = pSequenceKey->FindKey( pszFieldName );
	if( pData )
	{
		*flField = pData->GetFloat();
	}
}

bool CProgressSignScreenKeys::SetupStartupSequenecs() // Line 110
{
	if( !m_bStartupSequencesInitiailized )
	{
		ProgressSignStartupSequence_t defaultSequence;
		m_StartupSequences.AddToTail( defaultSequence );

		KeyValues *pKV = new KeyValues( "lightboard_startup_sequences" );
		if( pKV->LoadFromFile( filesystem, "scripts/vscripts/transitions/lightboard_startup_sequences.txt" ) )
		{
			KeyValues *pKeys = pKV->FindKey( "Startups" );
			if( pKeys )
			{
				for( KeyValues *pData = pKeys->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey() )
				{
					ProgressSignStartupSequence_t startupSequence;
					V_strcpy_safe( startupSequence.m_szName, pData->GetName() );
					SetSequenceFieldFromKey( pData, "flicker_rate_min", &startupSequence.m_flFlickerRateMin );
					SetSequenceFieldFromKey( pData, "flicker_rate_max", &startupSequence.m_flFlickerRateMax );
					SetSequenceFieldFromKey( pData, "flicker_quick_min", &startupSequence.m_flFlickerQuickMin );
					SetSequenceFieldFromKey( pData, "flicker_quick_max", &startupSequence.m_flFlickerQuickMax );
					SetSequenceFieldFromKey( pData, "bg_flicker_length", &startupSequence.m_flBGLength );
					SetSequenceFieldFromKey( pData, "level_number_delay", &startupSequence.m_flNumDelay );
					SetSequenceFieldFromKey( pData, "icon_delay", &startupSequence.m_flIconDelay );
					SetSequenceFieldFromKey( pData, "progress_delay", &startupSequence.m_flProgDelay );
					m_StartupSequences.AddToTail( startupSequence );
				}
			}
		}

		m_bStartupSequencesInitiailized = true;
		return true;
	}

	return false;
}

int CProgressSignScreenKeys::FindStartupSequenceIndex( const char *pszSequenceName ) // Line 171
{
	if( pszSequenceName )
	{
		FOR_EACH_VEC( m_StartupSequences, it )
		{
			if( FStrEq( m_StartupSequences[it].m_szName, pszSequenceName ) )
				return it;
		}
	}

	return 0;
}

int CProgressSignScreenKeys::GetRandomStartupSequenceIndex() // Line 194
{
	return RandomInt( 0, m_StartupSequences.Count() - 1 );
}

void CProgressSignScreenKeys::GetStartupSequences( int nIndex, ProgressSignStartupSequence_t *sequence ) // Line 203
{
	memcpy( sequence, &m_StartupSequences[nIndex], sizeof( ProgressSignStartupSequence_t ) );
}

KeyValues *CProgressSignScreenKeys::GetSPIconKeyValues() // Line 212
{
	return m_pPreloadedSPIconKeys;
}

KeyValues *CProgressSignScreenKeys::GetMPIconKeyValues() // Line 221
{
	return m_pPreloadedMPIconKeys;
}

CProgressSignScreenKeys g_ProgressSignScreenKeys( "CProgressSignScreenKeys" );



CVGUI_Base_ProgressSignScreen::CVGUI_Base_ProgressSignScreen( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName ) // Line 229
{
	m_nStartupSequenceIndex = 0;
	m_bStartingUp = false;
	m_nSU_Dirt_CurAlpha = 0;
	m_nDirtOverlayID = -1;
	m_nBackgroundTextureID = -1;
	m_nNumberTextureID = -1;
	m_nProgBarTextureID = -1;
	m_nIconsTextureID = -1;
	m_flSU_StartTime = 0.f;
	m_flSU_Length = 0.f;
	m_nSU_BG_CurAlpha = 0;
	m_flSU_IconAlphaScale = 0.f;
	m_bIsEnabledOld = false;
	m_flSU_FlickerRate = 0.f;
	m_flSU_LastFlicker = 0.f;
	m_flSU_FlickerRateQuick = 0.f;
	m_bIsAlreadyVisible = false;

	SetScheme( "basemodui_scheme" );

	m_nDirtOverlayTextures[0] = m_nDirtOverlayTextures[1] = m_nDirtOverlayTextures[2] = -1;

	ResetLevelData();
}

CVGUI_Base_ProgressSignScreen::~CVGUI_Base_ProgressSignScreen() // Line 254
{
	if( vgui::surface() )
	{
		if( m_nBackgroundTextureID != -1 )
			vgui::surface()->DestroyTextureID( m_nBackgroundTextureID );

		if( m_nNumberTextureID != -1 )
			vgui::surface()->DestroyTextureID( m_nNumberTextureID );

		if( m_nProgBarTextureID != -1 )
			vgui::surface()->DestroyTextureID( m_nProgBarTextureID );

		if( m_nIconsTextureID != -1 )
			vgui::surface()->DestroyTextureID( m_nIconsTextureID );

		for( int i = 0; i < NUM_DIRT_OVERLAYS; i++ )
		{
			if( m_nDirtOverlayTextures[i] != -1 )
				vgui::surface()->DestroyTextureID( m_nDirtOverlayTextures[i] );
		}
	}
}

void CVGUI_Base_ProgressSignScreen::ResetLevelData() // Line 281
{
	for( int i = 0; i < NUM_ICONS; i++ )
	{
		m_nDisplayedIcons[i] = -1;
		m_bHighlightedIcons[i] = false;
	}
}

bool CVGUI_Base_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData ) // Line 290
{
	if( m_nBackgroundTextureID == -1 )
	{
		m_nBackgroundTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nBackgroundTextureID, "vgui/screens/vgui_coop_progress_board", 1, false );
	}
	if( m_nNumberTextureID == -1 )
	{
		m_nNumberTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nNumberTextureID, "vgui/screens/vgui_coop_progress_board_numbers", 1, false );
	}
	if( m_nProgBarTextureID == -1 )
	{
		m_nProgBarTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nProgBarTextureID, "vgui/screens/vgui_coop_progress_board_bar", 1, false );
	}
	if( m_nIconsTextureID == -1 )
	{
		m_nIconsTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nIconsTextureID, "vgui/screens/P2_lightboard_vgui", 1, false );
	}
	if( m_nDirtOverlayTextures[0] == -1 )
	{
		m_nDirtOverlayTextures[0] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nDirtOverlayTextures[0], "vgui/elevator_video_overlay1", 1, false );
	}
	if( m_nDirtOverlayTextures[1] == -1 )
	{
		m_nDirtOverlayTextures[1] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nDirtOverlayTextures[1], "vgui/elevator_video_overlay2", 1, false );
	}
	if( m_nDirtOverlayTextures[2] == -1 )
	{
		m_nDirtOverlayTextures[2] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nDirtOverlayTextures[2], "vgui/elevator_video_overlay3", 1, false );
	}

	m_flGreyedIconAlpha = 134.f;
	m_nMaxBGAlpha = 140;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	if( pInitData && pInitData->m_pEntity )
	{
		C_VGuiScreen *pScreen = dynamic_cast<C_VGuiScreen *>( pInitData->m_pEntity );
		if( pScreen )
			m_bIsEnabledOld = pScreen->IsActive();
	}

	return BaseClass::Init( pKeyValues, pInitData );
}

void CVGUI_Base_ProgressSignScreen::OnTick() // Line 361
{
	BaseClass::OnTick();

	if( m_bIsEnabledOld || !IsEnabled() )
	{
		if( m_bStartingUp && gpGlobals->curtime > ( m_flSU_StartTime + m_flSU_Length ) )
			m_bStartingUp = false;
	}
	else
	{
		BeginStartupSequence();
	}

	m_bIsEnabledOld = IsEnabled();

	if( !m_bIsAlreadyVisible )
	{
		SetVisible( true );
		m_bIsAlreadyVisible = true;
	}
}

void CVGUI_Base_ProgressSignScreen::BeginStartupSequence() // Line 390
{
	m_flSU_StartTime = gpGlobals->curtime;
	m_flSU_LastFlicker = gpGlobals->curtime;
	m_bStartingUp = true;

	ProgressSignStartupSequence_t startupSequence;
	g_ProgressSignScreenKeys.GetStartupSequences( m_nStartupSequenceIndex, &startupSequence );

	m_flSU_FlickerRate = RandomFloat( startupSequence.m_flFlickerRateMin, startupSequence.m_flFlickerRateMax );
	m_flSU_FlickerRateQuick = RandomFloat( startupSequence.m_flFlickerQuickMin, startupSequence.m_flFlickerQuickMin ); // Min/Min?

	float m_flProgDelay = 0.f;
	if( startupSequence.m_flBGLength <= startupSequence.m_flProgDelay )
		m_flProgDelay = startupSequence.m_flProgDelay;
	else
		m_flProgDelay = startupSequence.m_flBGLength;

	float m_flIconDelay = 0.f;
	if( startupSequence.m_flNumDelay <= startupSequence.m_flIconDelay )
		m_flIconDelay = startupSequence.m_flIconDelay;
	else
		m_flIconDelay = startupSequence.m_flNumDelay;

	if( m_flProgDelay <= m_flIconDelay )
	{
		if( startupSequence.m_flNumDelay <= startupSequence.m_flIconDelay )
			m_flSU_Length = startupSequence.m_flIconDelay;
		else
			m_flSU_Length = startupSequence.m_flNumDelay;
	}
	else if( startupSequence.m_flBGLength <= startupSequence.m_flProgDelay )
	{
		m_flSU_Length = startupSequence.m_flProgDelay;
	}
	else
	{
		m_flSU_Length = startupSequence.m_flBGLength;
	}
}

float CVGUI_Base_ProgressSignScreen::UpdateStartupSequence() // Line 408
{
	m_flSU_IconAlphaScale = 1.f;
	float flCurNumAlpha = 255.f;

	ProgressSignStartupSequence_t startupSequence;
	g_ProgressSignScreenKeys.GetStartupSequences( m_nStartupSequenceIndex, &startupSequence );

	if( !m_bStartingUp )
	{
		m_nSU_BG_CurAlpha = m_nMaxBGAlpha;
		m_nSU_Dirt_CurAlpha = 255;
	}
	else
	{
		if( ( startupSequence.m_flBGLength + m_flSU_StartTime ) < gpGlobals->curtime )
		{
			m_nSU_BG_CurAlpha = m_nMaxBGAlpha;
			m_nSU_Dirt_CurAlpha = 255;
		}
		else
		{
			if( gpGlobals->curtime >= ( m_flSU_FlickerRateQuick + m_flSU_LastFlicker ) )
			{
				m_nSU_BG_CurAlpha = m_nMaxBGAlpha / 4;
				m_nSU_Dirt_CurAlpha = 63;
			}

			if( gpGlobals->curtime >= ( m_flSU_LastFlicker + m_flSU_FlickerRate ) )
			{
				m_nSU_BG_CurAlpha = ( m_nSU_BG_CurAlpha == m_nMaxBGAlpha ) ? m_nMaxBGAlpha / 4 : m_nMaxBGAlpha / 2;
				m_nSU_Dirt_CurAlpha = ( m_nSU_Dirt_CurAlpha != 255 ) ? 127 : 63;

				m_flSU_LastFlicker = gpGlobals->curtime;
				m_flSU_FlickerRate = RandomFloat( startupSequence.m_flFlickerRateMin, startupSequence.m_flFlickerRateMax );
				m_flSU_FlickerRateQuick = RandomFloat( startupSequence.m_flFlickerQuickMin, startupSequence.m_flFlickerQuickMin ); // Min/Min?
			}
		}

		if( ( m_flSU_StartTime + startupSequence.m_flNumDelay ) >= gpGlobals->curtime )
			flCurNumAlpha = 2.f;

		if( ( m_flSU_StartTime + startupSequence.m_flIconDelay ) >= gpGlobals->curtime )
			m_flSU_IconAlphaScale = 0.1f;
	}

	return flCurNumAlpha;
}

void CVGUI_Base_ProgressSignScreen::RenderBackground() // Line 471
{
	int nPanelWidth, nPanelHeight;
	GetSize( nPanelWidth, nPanelHeight );

	vgui::surface()->DrawSetColor( m_nSU_BG_CurAlpha, m_nSU_BG_CurAlpha, m_nSU_BG_CurAlpha, 255 );
	vgui::surface()->DrawSetTexture( m_nBackgroundTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, nPanelWidth, nPanelHeight );
}

void CVGUI_Base_ProgressSignScreen::RenderLevelNumber( int nLevelNumber, float flAlpha )
{
	vgui::surface()->DrawSetColor( 255, 255, 255, (int)flAlpha );
	vgui::surface()->DrawSetTexture( m_nNumberTextureID );

	int height = (int)( GetTall() * 0.4f );
	int width = height / 3;

	int y0 = height / 4;
	int y1 = y0 + height;

	// Draw tens digit
	int tensDigit = ( nLevelNumber / 10 ) % 10;
	float tensDigitT1 = (float)( tensDigit % 6 ) * 0.166f;
	float tensDigitT2 = tensDigitT1 + 0.166f;
	float tensDigitS = tensDigit >= 6 ? 0.5f : 0.0f;

	vgui::surface()->DrawTexturedSubRect( 70, y0, 70 + width, y1, tensDigitT1, tensDigitS, tensDigitT2, tensDigitS + 0.5f );

	// Draw units digit
	int unitsDigit = nLevelNumber % 10;
	float unitsDigitT1 = (float)( unitsDigit % 6 ) * 0.166f;
	float unitsDigitT2 = unitsDigitT1 + 0.166f;
	float unitsDigitS = unitsDigit >= 6 ? 0.5f : 0.0f;

	vgui::surface()->DrawTexturedSubRect( 70 + width, y0, 70 + 2 * width, y1, unitsDigitT1, unitsDigitS, unitsDigitT2, unitsDigitS + 0.5f );
}

void CVGUI_Base_ProgressSignScreen::RenderLevelProgress( int nCurrentLevel, int nTotalLevels, float flAlpha ) // Line 535
{
	// [PORTAL2BUG] NOTE(SanyaSho): here's original portal2 bug when the progressbar shows 12 segments on 9 level (check for DIST_EPSILON)
	if( nTotalLevels > 0 )
	{
		vgui::surface()->DrawSetColor( 255, 255, 255, (int)flAlpha );
		vgui::surface()->DrawSetTexture( m_nProgBarTextureID );

		float flLevel = 1.f;
		if( nCurrentLevel > 0 )
			flLevel = (float)nCurrentLevel / (float)nTotalLevels;

		float flWide = (float)( GetWide() + -162 );
		vgui::surface()->DrawTexturedSubRect( 81, 461, (int)( ( flWide * flLevel ) + 81.f ), 493, 0, 0, (float)( flWide * DIST_EPSILON ) * flLevel, 1.f );
	}
}

void CVGUI_Base_ProgressSignScreen::RenderIcons() // Line 560
{
	int nX0 = 81, nY0 = 577;

	for( int i = 0; i < NUM_ICONS; i++ )
	{
		int nIcon = m_nDisplayedIcons[i];
		if( nIcon == -1 )
			continue;

		float flPosX = (float)( ( nIcon - 1 ) % 5 ) * 0.2f;
		float flPosY = (float)( ( nIcon - 1 ) / 5 ) * 0.2f;

		// \n
		if( i == 5 )
		{
			nX0 = 81;
			nY0 += 63;
		}

		if( m_bHighlightedIcons[i] )
		{
			vgui::surface()->DrawSetColor( 0, 0, 0, 255 * m_flSU_IconAlphaScale );
		}
		else
		{
			vgui::surface()->DrawSetColor( 0, 0, 0, m_flGreyedIconAlpha * m_flSU_IconAlphaScale );
		}

		vgui::surface()->DrawSetTexture( m_nIconsTextureID );

		vgui::surface()->DrawTexturedSubRect( nX0, nY0, nX0 + 53, nY0 + 53, flPosX, flPosY, flPosX + 0.2f, flPosY + 0.2f );

		nX0 += 63;
	}
}

void CVGUI_Base_ProgressSignScreen::RenderDirtOverlays() // Line 604
{
	int nPanelWidth, nPanelHeight;
	GetSize( nPanelWidth, nPanelHeight );

	vgui::surface()->DrawSetColor( 255, 255, 255, m_nSU_Dirt_CurAlpha );
	vgui::surface()->DrawSetTexture( m_nDirtOverlayTextures[m_nDirtOverlayID] );
	vgui::surface()->DrawTexturedRect( 0, 0, nPanelWidth, nPanelHeight );
}

// SanyaSho: i don't know where i can put this (keep this on file end please)
const char *g_pszLightboardIcons[] =
{
	"dummy",
	"cube_drop",
	"cube_button",
	"cube_bonk",
	"drink_water",
	"goop",
	"crushers",
	"laser_cube",
	"turret",
	"turret_burn",
	"portal_fling",
	"plate_fling",
	"bridges",
	"bridge_block",
	"grinders",
	"tbeams",
	"paint_bounce",
	"paint_speed",
	"handoff",
	"button_stand",
	"laser_power",
	"portal_fling_2",
	"tbeam_polarity",
	"danger_field",
	NULL
};