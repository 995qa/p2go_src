//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "c_portal_player.h"
#include "c_weapon_portalgun.h"
#include "IGameUIFuncs.h"
#include "radialmenu.h"
#include "ivieweffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	HEALTH_WARNING_THRESHOLD	25


static ConVar	hud_quickinfo( "hud_quickinfo", "1", FCVAR_ARCHIVE );
static ConVar	hud_quickinfo_swap( "hud_quickinfo_swap", "0", FCVAR_ARCHIVE );

extern ConVar crosshair;

#define QUICKINFO_EVENT_DURATION	1.0f
#define	QUICKINFO_BRIGHTNESS_FULL	255
#define	QUICKINFO_BRIGHTNESS_DIM	64
#define	QUICKINFO_FADE_IN_TIME		0.5f
#define QUICKINFO_FADE_OUT_TIME		2.0f

/*
==================================================
CHUDQuickInfo 
==================================================
*/

using namespace vgui;

class CHUDQuickInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDQuickInfo, vgui::Panel );

public:
	CHUDQuickInfo( const char *pElementName );
	virtual ~CHUDQuickInfo();

	virtual void ApplySchemeSettings( IScheme *scheme );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint();

private:
	void DrawCrosshair( Color color, float flApparentZ );

	// SanyaSho: this code is ununsed in Linux bins and not exists in Windows bins
	//void DrawPortalHint( Vector vecPosition, bool bBluePortal );
	//void DrawPortalHints();

	//void DrawWarning();

	//bool EventTimeElapsed();

	//float m_flLastEventTime;
	// SanyaSho: end of unused code

	float m_fLastPlacedAlpha[2];
	bool m_bLastPlacedAlphaCountingUp[2];

	CHudTexture *m_icon_rbn; // right bracket
	CHudTexture *m_icon_lbn; // left bracket
	CHudTexture *m_icon_rb; // right bracket, full
	CHudTexture *m_icon_lb; // left bracket, full

	int m_nArrowTexture;
	int m_nCursorRadius;
	int m_nPortalIconOffsetX;
	int m_nPortalIconOffsetY;

	float m_flPortalIconScale;
};

DECLARE_HUDELEMENT( CHUDQuickInfo );

CHUDQuickInfo::CHUDQuickInfo( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HUDQuickInfo" )
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_CROSSHAIR );

	m_fLastPlacedAlpha[0] = m_fLastPlacedAlpha[1] = 0.0;
	m_bLastPlacedAlphaCountingUp[0] = m_bLastPlacedAlphaCountingUp[1] = true;

	m_nArrowTexture = -1;
	m_nCursorRadius = 0;
	m_nPortalIconOffsetX = m_nPortalIconOffsetY = 0;

	m_flPortalIconScale = 1.f;
}

CHUDQuickInfo::~CHUDQuickInfo()
{
	if( vgui::surface() && m_nArrowTexture != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nArrowTexture );
		m_nArrowTexture = -1;
	}
}

void CHUDQuickInfo::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_nCursorRadius = 10;
	m_nPortalIconOffsetX = 6;
	m_nPortalIconOffsetY = 10;
	m_flPortalIconScale = 1.f;

	SetPaintBackgroundEnabled( false );
}


void CHUDQuickInfo::Init( void )
{
	//m_flLastEventTime = 0.0; // used in EventTimeElapsed() which is unused

	if( m_nArrowTexture == -1 )
	{
		m_nArrowTexture = g_pVGuiSurface->CreateNewTextureID();
		g_pVGuiSurface->DrawSetTextureFile( m_nArrowTexture, "HUD/hud_icon_arrow", 1, 0 );
	}
}


void CHUDQuickInfo::VidInit( void )
{
	Init();

	m_icon_rb = HudIcons().GetIcon( "portal_crosshair_right_valid" );
	m_icon_lb = HudIcons().GetIcon( "portal_crosshair_left_valid" );
	m_icon_rbn = HudIcons().GetIcon( "portal_crosshair_right_invalid" );
	m_icon_lbn = HudIcons().GetIcon( "portal_crosshair_left_invalid" );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::ShouldDraw( void )
{
	if (!m_icon_rb || !m_icon_rbn || !m_icon_lb || !m_icon_lbn)
		return false;

	IClientMode *pClientMode = GetClientMode();
	if( !pClientMode || !pClientMode->ShouldDrawCrosshair() )
		return false;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( player == NULL )
		return false;

	if ( !crosshair.GetBool() )
		return false;

	if( IsRadialMenuOpen() )
		return false;

	if( ( player->GetPlayerRenderMode( engine->GetActiveSplitScreenPlayerSlot() ) == PLAYER_RENDER_THIRDPERSON ) || player->GetViewEntity() && player->GetViewEntity() != player )
		return false;

	return ( CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage() );
}

void CHUDQuickInfo::Paint()
{
	C_BasePlayer *pPlayer = GetSplitScreenViewPlayer( engine->GetActiveSplitScreenPlayerSlot() );
	if( !pPlayer )
		return;

	// TODO: color[3] is used in pair with other value somehere...
	byte color[4];
	bool blend;
	GetViewEffects()->GetFadeParams( &color[0], &color[1], &color[2], &color[3], &blend );

	Vector pos, fwd, right, up;
	pPlayer->EyePositionAndVectors( &pos, &fwd, &right, &up );

	trace_t tr;
	Ray_t ray;
	ray.Init( pos, fwd * 56755.84 + pos ); // TODO: figure out what the fuck is 56755.84
	CTraceFilterSimple filter( pPlayer, 0, 0 );
	UTIL_Portal_TraceRay( ray, MASK_SHOT, &filter, &tr );

	float flApparentZ = ( tr.fraction < 1.0f ) ? ( pos - tr.endpos ).Length() : MAX_TRACE_LENGTH;
	DrawCrosshair( Color( 255, 255, 255, 255 ), flApparentZ );

	// Don't draw the portal indicators if hud_quickinfo is 0
	if( !hud_quickinfo.GetBool() )
		return;

	if( !pPlayer->GetActiveWeapon() )
		return;

	C_WeaponPortalgun *pPortalGun = dynamic_cast<C_WeaponPortalgun * >( pPlayer->GetActiveWeapon() );
	if( pPortalGun && ( pPortalGun->CanFirePortal1() || pPortalGun->CanFirePortal2() ) )
	{
		bool bPortalPlacability[2] = { false, false };

		if( pPortalGun->GetAssociatedPortal( false ) )
			bPortalPlacability[0] = pPortalGun->GetAssociatedPortal( false )->IsActive();

		if( pPortalGun->GetAssociatedPortal( true ) )
			bPortalPlacability[1] = pPortalGun->GetAssociatedPortal( true )->IsActive();

		Color portal1Color = UTIL_Portal_Color( 1, pPlayer->GetTeamNumber() );
		Color portal2Color = UTIL_Portal_Color( 2, pPlayer->GetTeamNumber() );

		portal1Color[3] = 128;
		portal2Color[3] = 128;

		const int iBaseLastPlacedAlpha = 128;
		Color lastPlaced1Color = Color( portal1Color[0], portal1Color[1], portal1Color[2], iBaseLastPlacedAlpha );
		Color lastPlaced2Color = Color( portal2Color[0], portal2Color[1], portal2Color[2], iBaseLastPlacedAlpha );

		const float fLastPlacedAlphaLerpSpeed = 300.0f;

		float fLeftPlaceBarFill = 0.0f;
		float fRightPlaceBarFill = 0.0f;

		if( pPortalGun->CanFirePortal1() && pPortalGun->CanFirePortal2() )
		{
			int iDrawLastPlaced = 0;

			//do last placed indicator effects
			if( pPortalGun->GetLastFiredPortal() == 1 )
			{
				iDrawLastPlaced = 0;
				fLeftPlaceBarFill = 1.0f;
			}
			else if( pPortalGun->GetLastFiredPortal() == 2 )
			{
				iDrawLastPlaced = 1;
				fRightPlaceBarFill = 1.0f;
			}

			if( m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] )
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] += gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed * 2.0f;
				if( m_fLastPlacedAlpha[iDrawLastPlaced] > 255.0f )
				{
					m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] = false;
					m_fLastPlacedAlpha[iDrawLastPlaced] = 255.0f - ( m_fLastPlacedAlpha[iDrawLastPlaced] - 255.0f );
				}
			}
			else
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
				if( m_fLastPlacedAlpha[iDrawLastPlaced] < (float)iBaseLastPlacedAlpha )
				{
					m_fLastPlacedAlpha[iDrawLastPlaced] = (float)iBaseLastPlacedAlpha;
				}
			}

			//reset the last placed indicator on the other side
			m_fLastPlacedAlpha[1 - iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
			if( m_fLastPlacedAlpha[1 - iDrawLastPlaced] < 0.0f )
			{
				m_fLastPlacedAlpha[1 - iDrawLastPlaced] = 0.0f;
			}
			m_bLastPlacedAlphaCountingUp[1 - iDrawLastPlaced] = true;

			if( pPortalGun->GetLastFiredPortal() != 0 )
			{
				lastPlaced1Color[3] = m_fLastPlacedAlpha[0];
				lastPlaced2Color[3] = m_fLastPlacedAlpha[1];
			}
			else
			{
				lastPlaced1Color[3] = 0.0f;
				lastPlaced2Color[3] = 0.0f;
			}
		}
		//can't fire both portals, and we want the crosshair to remain somewhat symmetrical without being confusing
		else if( !pPortalGun->CanFirePortal1() )
		{
			// clone portal2 info to portal 1
			portal1Color = portal2Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[0] = bPortalPlacability[1];
		}
		else if( !pPortalGun->CanFirePortal2() )
		{
			// clone portal1 info to portal 2
			portal2Color = portal1Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[1] = bPortalPlacability[0];
		}

		// TODO: m_flPortalIconScale should be used with icon->EffectiveWidht/Height
		int xCenter = ScreenWidth() / 2;
		int yCenter = ( ScreenHeight() - m_icon_lb->Height() ) / 2; // TODO: this should be removed

		if( !hud_quickinfo_swap.GetBool() )
		{
			if( bPortalPlacability[0] )
				m_icon_lb->DrawSelf( xCenter - ( m_icon_lb->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal1Color );
			else
				m_icon_lbn->DrawSelf( xCenter - ( m_icon_lbn->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal1Color );

			if( bPortalPlacability[1] )
				m_icon_rb->DrawSelf( xCenter + ( m_icon_rb->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal2Color );
			else
				m_icon_rbn->DrawSelf( xCenter + ( m_icon_rbn->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal2Color );
		}
		else
		{
			if( bPortalPlacability[1] )
				m_icon_lb->DrawSelf( xCenter - ( m_icon_lb->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal2Color );
			else
				m_icon_lbn->DrawSelf( xCenter - ( m_icon_lbn->Width() * 0.64f ), yCenter - ( m_icon_rb->Height() * 0.17f ), portal2Color );

			if( bPortalPlacability[0] )
				m_icon_rb->DrawSelf( xCenter + ( m_icon_rb->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal1Color );
			else
				m_icon_rbn->DrawSelf( xCenter + ( m_icon_rbn->Width() * -0.35f ), yCenter + ( m_icon_rb->Height() * 0.17f ), portal1Color );
		}
	}
}

void CHUDQuickInfo::DrawCrosshair( Color color, float flApparentZ )
{
	g_pVGuiSurface->DrawSetAlphaMultiplier( flApparentZ );
	g_pVGuiSurface->DrawSetColor( color );

	int nWidth = ( ScreenWidth() / 2 );
	int nHeight = ( ScreenHeight() / 2 );

	g_pVGuiSurface->DrawFilledRect( nWidth, nHeight, nWidth + 1, nHeight + 1 );
	g_pVGuiSurface->DrawFilledRect( nWidth + m_nCursorRadius, nHeight, nWidth + m_nCursorRadius + 1, nHeight + 1 );
	g_pVGuiSurface->DrawFilledRect( nWidth - m_nCursorRadius, nHeight, nWidth - m_nCursorRadius + 1, nHeight + 1 );
	g_pVGuiSurface->DrawFilledRect( nWidth, nHeight + m_nCursorRadius, nWidth + 1, nHeight + m_nCursorRadius + 1 );
	g_pVGuiSurface->DrawFilledRect( nWidth, nHeight - m_nCursorRadius, nWidth + 1, nHeight - m_nCursorRadius + 1 );
	g_pVGuiSurface->DrawClearApparentDepth();
}
