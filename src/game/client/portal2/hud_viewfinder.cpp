//================= Copyright Valve Corporation, All rights reserved. =================//
//
// Purpose: F-Stop camera zoom HUD.
// Notes: SanyaSho: this hudelement was reused in Co-Op.
//
//=====================================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "vgui_controls/Panel.h"
#include "clientmode_shared.h"
#include "c_portal_player.h"
#include "view_scene.h"
#include "vgui/ILocalize.h"

// gay
#include "tier0/memdbgon.h"

extern int ScreenTransform( const Vector &point, Vector &screen ); // Line ???

class CHudViewfinder : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudViewfinder, vgui::Panel );

public:
	CHudViewfinder( const char *pElementName );
	virtual ~CHudViewfinder();

	virtual void Init();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual bool ShouldDraw();
	virtual void Paint();

private:
	int m_iScopeTexture[2];
};

DECLARE_HUDELEMENT_DEPTH( CHudViewfinder, 100 );

CHudViewfinder::CHudViewfinder( const char *pElementName ) : BaseClass( NULL, "HudViewfinder" ), CHudElement( pElementName ) // Line 65
{
	SetParent( GetClientMode()->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_iScopeTexture[0] = m_iScopeTexture[1] = -1;
}

CHudViewfinder::~CHudViewfinder() // Line 78
{
}

void CHudViewfinder::Init() // Line 97
{
	if( m_iScopeTexture[0] == -1 )
	{
		m_iScopeTexture[0] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iScopeTexture[0], "HUD/camera_viewfinder_ul", 1, 0 );
	}

	if( m_iScopeTexture[1] == -1 )
	{
		m_iScopeTexture[1] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iScopeTexture[1], "HUD/camera_viewfinder_halfcircle", 1, 0 );
	}

	UnregisterForRenderGroup( "global" );
}

void CHudViewfinder::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 117
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	int wide, tall;
	GetHudSize( wide, tall );
	SetBounds( 0, 0, wide, tall );

	SetVisible( false );
}

bool ShouldDrawHudViewfinder() // Line 132
{
	CPortal_Player *pLocalPlayer = CPortal_Player::GetLocalPortalPlayer();
	return pLocalPlayer && pLocalPlayer->IsTaunting() && ( pLocalPlayer->IsRemoteViewTaunt() || pLocalPlayer->IsShowingViewFinder() );
}

bool CHudViewfinder::ShouldDraw() // Line 142
{
	return ShouldDrawHudViewfinder();
}

void UTIL_WorldToScreenCoords( const Vector &vecWorld, int *pScreenX, int *pScreenY ) // Line 149
{
	*pScreenX = 0;
	*pScreenY = 0;

	Vector vecTransform;
	if( ScreenTransform( vecWorld, vecTransform ) )
		return;

	*pScreenX = ( ScreenWidth() / 2 ) + 0.5f * vecTransform.x * ScreenWidth() + 0.5f;
	*pScreenY = ( ScreenHeight() / 2 ) - 0.5f * vecTransform.y * ScreenHeight() + 0.5f;
}

void UTIL_GenerateBoxVertices( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, Vector pVerts[8] ); // Line ???
bool UTIL_WorldSpaceToScreensSpaceBounds( const Vector &vecCenter, const Vector &mins, const Vector &maxs, Vector2D *pMins, Vector2D *pMaxs ) // Line 174
{
	Vector vecBoxVerts[8];
	UTIL_GenerateBoxVertices( vecCenter, mins, maxs, vecBoxVerts );

	int nMaxX = 0;
	int nMinX = ScreenWidth();
	int nMaxY = 0;
	int nMinY = ScreenHeight();

	int nX, nY;
	for( int i = 0; i < 8; i++ )
	{
		UTIL_WorldToScreenCoords( vecBoxVerts[i], &nX, &nY );

#if 0
		surface()->DrawSetColor( 255, 128, 0, 255 );
		surface()->DrawOutlinedCircle( nX, nY, 4, 4 );
#endif

		if( nX > nMaxX )
		{
			nMaxX = nX;
		}
		else if( nX < nMinX )
		{
			nMinX = nX;
		}

		if( nY > nMaxY )
		{
			nMaxY = nY;
		}
		else if( nY < nMinY )
		{
			nMinY = nY;
		}
	}

#if 0
	surface()->DrawSetColor( 255, 0, 0, 255 );
	surface()->DrawOutlinedCircle( nMaxX, nMaxY, 8, 32 );

	surface()->DrawSetColor( 0, 255, 0, 255 );
	surface()->DrawOutlinedCircle( nMinX, nMinY, 8, 32 );
#endif // 0

	if( pMins != NULL )
	{
		pMins->x = nMinX;
		pMins->y = nMinY;
	}

	if( pMaxs != NULL )
	{
		pMaxs->x = nMaxX;
		pMaxs->y = nMaxY;
	}

	return true;
}

void UTIL_GenerateBoxVertices( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, Vector pVerts[8] ) // Line 162
{
	Vector vecPos;
	for( int i = 0; i < 8; ++i )
	{
		vecPos[0] = ( i & 0x1 ) ? vMaxs[0] : vMins[0];
		vecPos[1] = ( i & 0x2 ) ? vMaxs[1] : vMins[1];
		vecPos[2] = ( i & 0x4 ) ? vMaxs[2] : vMins[2];
		pVerts[i] = vecPos + vOrigin;
	}
}

bool UTIL_EntityBoundsToSizes( C_BaseEntity *pTarget, int *pMinX, int *pMinY, int *pMaxX, int *pMaxY ) // Line 236
{
	Vector vOBBMins, vOBBMaxs;
	pTarget->CollisionProp()->WorldSpaceSurroundingBounds( &vOBBMins, &vOBBMaxs );
	vOBBMaxs -= pTarget->GetAbsOrigin();
	vOBBMins -= pTarget->GetAbsOrigin();

	Vector2D mins, maxs;
	UTIL_WorldSpaceToScreensSpaceBounds( pTarget->GetAbsOrigin(), vOBBMins, vOBBMaxs, &mins, &maxs );

	if( pMinX )
		*pMinX = mins.x;

	if( pMinY )
		*pMinY = mins.y;

	if( pMaxX )
		*pMaxX = maxs.x;

	if( pMaxY )
		*pMaxY = maxs.y;

	return true;
}

void CHudViewfinder::Paint() // Line 353
{
	// goddamn vertices...
	UpdateRefractTexture();

	int screenWide, screenTall;
	GetHudSize( screenWide, screenTall );

	int wide = screenWide / 2;
	int tall = screenTall / 2;

	vgui::surface()->DrawSetColor( 0, 0, 0, 255 );

	vgui::surface()->DrawSetTexture( m_iScopeTexture[0] );

	float uv1 = 0.0f, uv2 = 1.0f;
	Vector2D uv11( uv1, uv1 );
	Vector2D uv21( uv2, uv1 );
	Vector2D uv22( uv2, uv2 );
	Vector2D uv12( uv1, uv2 );

	vgui::Vertex_t vert[4];

	/// NOTE: Linux: v7 - wide; v8 - tall; xRighta - wide * 2; v17 - tall

	// Top-left
	vert[0].Init( Vector2D( 0, 0 ), uv11 );
	vert[1].Init( Vector2D( wide, 0 ), uv21 );
	vert[2].Init( Vector2D( wide, tall ), uv22 );
	vert[3].Init( Vector2D( 0, tall ), uv12 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Top-right
	vert[0].Init( Vector2D( wide, 0 ), uv21 );
	vert[1].Init( Vector2D( wide * 2, 0 ), uv11 );
	vert[2].Init( Vector2D( wide * 2, tall ), uv12 );
	vert[3].Init( Vector2D( wide, tall ), uv22 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Bottom-right
	vert[0].Init( Vector2D( wide, tall ), uv22 );
	vert[1].Init( Vector2D( wide * 2, tall ), uv12 );
	vert[2].Init( Vector2D( wide * 2, screenTall ), uv11 );
	vert[3].Init( Vector2D( wide, screenTall ), uv21 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Bottom-left
	vert[0].Init( Vector2D( 0, tall ), uv12 );
	vert[1].Init( Vector2D( wide, tall ), uv22 );
	vert[2].Init( Vector2D( wide, screenTall ), uv21 );
	vert[3].Init( Vector2D( 0, screenTall ), uv11 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	int panelWide, panelTall;
	GetSize( panelWide, panelTall );

	/// NOTE: Windows: v2 - wide; v4 - tall;

	float flCircleScale = screenTall * 0.15f;
	vgui::surface()->DrawOutlinedCircle( wide, tall, flCircleScale, 64 );
	vgui::surface()->DrawOutlinedCircle( wide, tall, flCircleScale * 1.25f, 64 );

	flCircleScale = screenTall * 0.05f;
	
	// Change texture to grill circle
	vgui::surface()->DrawSetTexture( m_iScopeTexture[1] );

	/// NOTE: Linux: v7 - wide; v8 - tall; xRighta - wide * 2; v17 - tall; v9 - flCircleScale

	// Bottom
	vgui::surface()->DrawSetColor( 0, 0, 0, 64 );
	vert[0].Init( Vector2D( wide - flCircleScale, tall ), uv12 );
	vert[1].Init( Vector2D( wide + flCircleScale, tall ), uv22 );
	vert[2].Init( Vector2D( wide + flCircleScale, tall + flCircleScale ), uv21 );
	vert[3].Init( Vector2D( wide - flCircleScale, tall + flCircleScale ), uv11 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Top
	vgui::surface()->DrawSetColor( 0, 0, 0, 32 );
	vert[0].Init( Vector2D( wide - flCircleScale, tall - flCircleScale ), uv21 );
	vert[1].Init( Vector2D( wide + flCircleScale, tall - flCircleScale ), uv11 );
	vert[2].Init( Vector2D( wide + flCircleScale, tall ), uv12 );
	vert[3].Init( Vector2D( wide - flCircleScale, tall ), uv22 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Reset alpha
	vgui::surface()->DrawSetColor( 0, 0, 0, 255 );

	/// NOTE: Linux: v7 - wide; v8 - tall; xRighta - wide * 2; v17 - tall; v9 - flCircleScale

	flCircleScale = screenWide * 0.1f;

	int flPos = screenWide * 0.125f;

	vgui::surface()->DrawFilledRect(
		flPos,
		flPos,
		flCircleScale + flPos,
		flPos + 1.f
	);

	vgui::surface()->DrawFilledRect(
		flPos,
		flPos,
		flPos + 1.f,
		flCircleScale + flPos
	);

	/// NOTE: v12 - flPos; flRegisterOffset - flPos; v11 - (flCircleScale + flPos); 
	/// v33 - (flCircleScale + flPos); v10 - flPos + 1.f; xLeft - flPos + 1.f;

	vgui::surface()->DrawFilledRect(
		flPos,
		screenTall - flPos,
		flCircleScale + flPos,
		( screenTall - flPos ) + 1.f
	);

	vgui::surface()->DrawFilledRect(
		flPos,
		screenTall - ( flCircleScale + flPos ),
		flPos + 1.f,
		screenTall - flPos
	);

	vgui::surface()->DrawFilledRect(
		screenWide - ( flCircleScale + flPos ),
		flPos,
		screenWide - flPos,
		flPos + 1.f
	);

	vgui::surface()->DrawFilledRect(
		screenWide - ( flPos + 1.f ),
		flPos,
		screenWide - flPos,
		flPos + 1.f
	);

	/// NOTE: v12 - flPos; flRegisterOffset - flPos; v11 - (flCircleScale + flPos); 
	/// v33 - (flCircleScale + flPos); v10 - flPos + 1.f; xLeft - flPos + 1.f;

	vgui::surface()->DrawFilledRect(
		screenWide - ( flPos + 1.f ),
		flPos,
		screenWide - flPos,
		flCircleScale + flPos
	);

	vgui::surface()->DrawFilledRect(
		screenWide - ( flCircleScale + flPos ),
		screenTall - ( flPos + 1.f ),
		screenWide - flPos,
		screenTall - flPos
	);

	vgui::surface()->DrawFilledRect(
		screenWide - ( flPos + 1.f ),
		screenTall - ( flCircleScale + flPos ),
		screenWide - flPos,
		screenTall - flPos
	);

	// [PORTAL2BUG]: InstructorTitle font is located in the basemodui_scheme scheme.
	vgui::HScheme hScheme = vgui::scheme()->GetScheme( "ClientScheme" );

	vgui::HFont hFont = vgui::scheme()->GetIScheme( hScheme )->GetFont( "InstructorTitle", true );

	vgui::surface()->DrawSetTextFont( hFont );

	// TODO: something with sinf() gpGlobals->curtime * 8.f
	vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, 255 ) );

	wchar_t unicode[256];
	g_pVGuiLocalize->ConvertANSIToUnicode( "REC", unicode, 512 );

	int nWide, nTall;
	vgui::surface()->GetTextSize( hFont, unicode, nWide, nTall );

	vgui::surface()->DrawSetTextPos(
		( screenWide - flPos ) - ( nWide + 32 ),
		( screenTall - flPos ) - ( nTall + 16 )
	);

	vgui::surface()->DrawUnicodeString( unicode, FONT_DRAW_DEFAULT );
}
