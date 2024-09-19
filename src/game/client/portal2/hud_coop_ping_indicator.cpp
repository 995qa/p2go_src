#include "cbase.h"
#include "hudelement.h"
#include "vgui_controls/Panel.h"
#include "clientmode_shared.h"
#include "usermessages.h"
#include "hud_macros.h"
#include "view_scene.h"

// gay
#include "tier0/memdbgon.h"

// TODO: CONVARS
extern ConVar sv_portal_coop_ping_hud_indicitator_duration;
ConVar cl_coop_ping_indicator_scale( "cl_coop_ping_indicator_scale", "1" );

class CHudCoopPingIndicator : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudCoopPingIndicator, vgui::Panel );

public:
	CHudCoopPingIndicator( const char *pElementName );
	virtual ~CHudCoopPingIndicator();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ); // Line 74
	virtual void Init(); // Line 82
	virtual void LevelInit(); // Line 94
	virtual bool ShouldDraw(); // Line 104

	void DrawIndicatorHint(); // Line 122

	virtual void Paint(); // Line 192

	bool MsgFunc_HudPingIndicator( const CUsrMsg_HudPingIndicator &msg ); // Line 197

private:
	CUserMessageBinder m_UMCMsgHudPingIndicator;

	float m_flStartDisplayTime;
	Vector m_vecPingLocation;
	int m_nArrowTexture;
};

DECLARE_HUDELEMENT( CHudCoopPingIndicator );
DECLARE_HUD_MESSAGE( CHudCoopPingIndicator, HudPingIndicator );

CHudCoopPingIndicator::CHudCoopPingIndicator( const char *pElementName ) : BaseClass( NULL, "HudCoopPingIndicator" ), CHudElement( pElementName ) // Line 55
{
	SetParent( GetClientMode()->GetViewport() );

	m_nArrowTexture = -1;
	
	// Was uninitialized
	//m_vecPingLocation.Init();
	m_flStartDisplayTime = 0.f;
}

CHudCoopPingIndicator::~CHudCoopPingIndicator() // Line 64
{
}

void CHudCoopPingIndicator::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 74
{
	BaseClass::ApplySchemeSettings( pScheme );
	// (*(void (__cdecl **)(vgui::Panel *, _DWORD))(*(_DWORD *)this->gap0 + 288))(&this->vgui::Panel, 0);
}

void CHudCoopPingIndicator::Init() // Line 82
{
	if( this->m_nArrowTexture == -1 )
	{
		m_nArrowTexture = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_nArrowTexture, "HUD/hud_icon_arrow", 1, false );
	}

	HOOK_HUD_MESSAGE( CHudCoopPingIndicator, HudPingIndicator );
}

void CHudCoopPingIndicator::LevelInit() // Line 94
{
	m_flStartDisplayTime = -1.f - sv_portal_coop_ping_hud_indicitator_duration.GetFloat();
}

bool CHudCoopPingIndicator::ShouldDraw() // Line 104
{
	if( !C_BasePlayer::GetLocalPlayer()
	|| !g_pGameRules->IsMultiplayer()
	|| gpGlobals->curtime > sv_portal_coop_ping_hud_indicitator_duration.GetFloat() + m_flStartDisplayTime )
	{
		return false;
	}

	return CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage();
}

extern void UTIL_WorldToScreenCoords( const Vector &vecWorld, int *pScreenX, int *pScreenY );

void CHudCoopPingIndicator::DrawIndicatorHint() // Line 122
{
	if( m_vecPingLocation == vec3_invalid )
		return;

	//Vector vecScreen;
	//ScreenTransform( m_vecPingLocation, vecScreen );

	// int nScreenX, nScreenY;
	// UTIL_WorldToScreenCoords( m_vecPingLocation, &nScreenX, &nScreenY );

	//int nXMid = ScreenWidth() / 2;
	//int nYMid = ScreenHeight() / 2;

	float uv1 = 0.0f, uv2 = 1.0f;
	Vector2D uv11( uv1, uv1 );
	Vector2D uv21( uv2, uv1 );
	Vector2D uv22( uv2, uv2 );
	Vector2D uv12( uv1, uv2 );

	float flScale = cl_coop_ping_indicator_scale.GetFloat();

	vgui::Vertex_t vert[4];
	vert[0].Init( Vector2D( 10, 10 ), uv11 );
	vert[1].Init( Vector2D( 10, 10 ), uv21 );
	vert[2].Init( Vector2D( 10, 10 ), uv22 );
	vert[3].Init( Vector2D( 10, 10 ), uv12 );

/*
	v29 = atan2(v19, v20);
    v27 = sin(v29);
    v28 = cos(v29);

	v10 = (-0.5 * flScale) * v28;
	v11 = (-0.5 * flScale) * v27;
	v12 = ( (v7 + ( v28 * v21 ) ) + v10 ) - v11;
	v13 = -v27 * flScale;

	verts[0].m_Position.x = v12;
	verts[0].m_Position.y = ( ( v8 + ( v21 * v27 ) ) + v11 ) + v10;

	verts[1].m_Position.x = v12 + (v28 * flScale);
	verts[1].m_Position.y = (v27 * flScale) + verts[0].m_Position.y;

	verts[2].m_Position.x = verts[1].m_Position.x + v13;
	verts[2].m_Position.y = verts[1].m_Position.y + (v28 * flScale);

	verts[3].m_Position.x = v13 + v12;
	verts[3].m_Position.y = (v28 * flScale) + verts[0].m_Position.y;
*/

	// TODO: draw the arrow texture
}

void CHudCoopPingIndicator::Paint() // Line 192
{
	DrawIndicatorHint();
}

bool CHudCoopPingIndicator::MsgFunc_HudPingIndicator( const CUsrMsg_HudPingIndicator &msg ) // Line 197
{
	if( !C_BasePlayer::GetLocalPlayer() )
		return false;

	m_flStartDisplayTime = gpGlobals->curtime;

	m_vecPingLocation.x = msg.posx();
	m_vecPingLocation.y = msg.posy();
	m_vecPingLocation.z = msg.posz();

	return true;
}