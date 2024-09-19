//================= Copyright Valve Corporation, All rights reserved. =================//
//
// Purpose: F-Stop placement indicator HUD.
// Notes: 
//
//=====================================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"
#include "clientmode_shared.h"
#include "usermessages.h"
#include "hud_macros.h"

// gay
#include "tier0/memdbgon.h"

class CHudIndicator : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudIndicator, vgui::EditablePanel );

public:
	CHudIndicator( const char *pElementName );
	virtual ~CHudIndicator();

	virtual void Init();
	virtual void Reset();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual bool ShouldDraw();
	virtual void Paint();

	// Camera usermessages
	bool MsgFunc_IndicatorFlash( const CUsrMsg_IndicatorFlash &msg );

private:
	CUserMessageBinder m_UMCMsgIndicatorFlash;

	int m_nTexture[2];
	int m_nIndicatorType;

	float m_flDisplayTime;
	float m_flStartTime;
	float m_flFadeInTime;
	float m_flFadeOutTime;	
};

// TODO(SanyaSho): was disabled in retail P2?
DECLARE_HUDELEMENT( CHudIndicator );
DECLARE_HUD_MESSAGE( CHudIndicator, IndicatorFlash );

CHudIndicator::CHudIndicator( const char *pElementName ) : BaseClass( NULL, "HudIndicator" ), CHudElement( pElementName ) // Line 63
{
	SetParent( GetClientMode()->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	LoadControlSettings( "Resource/Indicator.res" );

	m_nTexture[0] = m_nTexture[1] = -1;
	m_nIndicatorType = 0;

	// Was uninitialized
	m_flDisplayTime = m_flStartTime = m_flFadeInTime = m_flFadeOutTime = 0.f;
}

CHudIndicator::~CHudIndicator() // Line 80
{
	if( m_nTexture[0] != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nTexture[0] );
		m_nTexture[0] = -1;
	}

	if( m_nTexture[1] != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nTexture[1] );
		m_nTexture[1] = -1;
	}
}

void CHudIndicator::Init() // Line 99
{
	HOOK_HUD_MESSAGE( CHudIndicator, IndicatorFlash );

	if( m_nTexture[0] == -1 )
	{
		m_nTexture[0] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nTexture[0], "HUD/invalid", 1, false );
	}

	if( m_nTexture[1] == -1 )
	{
		m_nTexture[1] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nTexture[1], "HUD/inv_full", 1, false );
	}

	m_flDisplayTime = 0.f;
	m_flStartTime = 0.f;
	m_flFadeInTime = 0.f;
	m_flFadeOutTime = 0.f;
}

void CHudIndicator::Reset() // Line 124
{
	// SanyaSho: i'm not sure what we should put here.
	m_flDisplayTime = 0.f;
	m_flStartTime = 0.f;
	m_flFadeInTime = 0.f;
	m_flFadeOutTime = 0.f;
}

void CHudIndicator::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 132
{
	LoadControlSettings( "Resource/Indicator.res" );
	
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	int wide, tall;
	GetHudSize( wide, tall );
	SetBounds( ( wide / 2 ) - 128, ( tall / 2 ) - 128, 256, 256 );
}

bool CHudIndicator::ShouldDraw() // Line 153
{
	return m_flDisplayTime > gpGlobals->curtime;
}

void CHudIndicator::Paint() // Line 163
{
	float flGlobalAlpha = 0.f;

	float flCurTime = gpGlobals->curtime;

	// TODO: Need to simplify this stuff somehow...
	if( m_flFadeInTime > flCurTime )
	{
		if( m_flFadeInTime == m_flStartTime )
		{
			flGlobalAlpha = MIN( m_flFadeInTime, flCurTime );
		}
		else
		{
			float progress = ( flCurTime - m_flStartTime ) / ( m_flFadeInTime - m_flStartTime );
			if( progress >= 0.f )
			{
				flGlobalAlpha = 1.f;
				if( progress <= 1.f )
				{
					flGlobalAlpha = ( 3.f * progress * progress ) - ( 2.f * progress * progress * progress );
				}
			}
		}
	}
	else
	{
		if( flCurTime <= m_flFadeOutTime )
		{
			flGlobalAlpha = 1.f;
		}
		else
		{
			if( m_flFadeOutTime == m_flDisplayTime )
			{
				flGlobalAlpha = MIN( m_flDisplayTime, flCurTime );
			}
			else
			{
				float progress = ( flCurTime - m_flFadeOutTime ) / ( m_flDisplayTime - m_flFadeOutTime );
				if( progress >= 0.f )
				{
					flGlobalAlpha = 0.f;
					if( progress <= 1.f )
					{
						flGlobalAlpha = 1.f - ( ( 3.f * progress * progress ) - ( 2.f * progress * progress * progress ) );
					}
				}
			}
		}
	}

	// TODO: Need to simplify this stuff somehow...
	float v17 = ( ( ( ( sinf( flCurTime * 10.f ) * 16.0 ) * flGlobalAlpha ) * flGlobalAlpha ) + 128.0 ) * 0.5;

	vgui::surface()->DrawSetTexture( m_nTexture[m_nIndicatorType] );

	vgui::surface()->DrawSetColor( 0, 0, 0, (int)( 128.f * flGlobalAlpha ) );

	// TODO: how i should call these floats?
	float v19 = ( GetWide() / 2 ) - v17;
	float v16 = ( GetWide() / 2 ) + v17;
	float v15 = ( GetTall() / 2 ) - v17;
	float v18 = ( GetTall() / 2 ) + v17;

	float uv1 = 0.0f, uv2 = 1.0f;
	Vector2D uv11( uv1, uv1 );
	Vector2D uv21( uv2, uv1 );
	Vector2D uv22( uv2, uv2 );
	Vector2D uv12( uv1, uv2 );

	vgui::Vertex_t vert[4];
	vert[0].Init( Vector2D( v19 + 4.f, v15 + 4.f ), uv11 );
	vert[1].Init( Vector2D( v16 + 4.f, v15 + 4.f ), uv21 );
	vert[2].Init( Vector2D( v16 + 4.f, v18 + 4.f ), uv22 );
	vert[3].Init( Vector2D( v19 + 4.f, v18 + 4.f ), uv12 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	vgui::surface()->DrawSetColor( 255, 255, 255, (int)( flGlobalAlpha * 255.f ) );

	vert[0].Init( Vector2D( v19, v15 ), uv11 );
	vert[1].Init( Vector2D( v16, v15 ), uv21 );
	vert[2].Init( Vector2D( v16, v18 ), uv22 );
	vert[3].Init( Vector2D( v19, v18 ), uv12 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	BaseClass::Paint();
}

bool CHudIndicator::MsgFunc_IndicatorFlash( const CUsrMsg_IndicatorFlash &msg ) // Line 220
{
	m_nIndicatorType = msg.indicator_type();

	m_flStartTime = gpGlobals->curtime;
	float flDisplayTime = gpGlobals->curtime + msg.fadetime();
	m_flDisplayTime = flDisplayTime;
	m_flFadeInTime = gpGlobals->curtime + 0.2f;
	m_flFadeOutTime = flDisplayTime - 0.2f;

	return true;
}