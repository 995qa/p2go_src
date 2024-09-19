#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_controls/EditablePanel.h"
#include "clientmode_shared.h"
#include "portal_mp_gamerules.h"
#include "c_team.h"
#include "c_playerresource.h"

// gay
#include "tier0/memdbgon.h"

class CHudMultiplayerBasicInfo : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudMultiplayerBasicInfo, vgui::EditablePanel );

public:
	CHudMultiplayerBasicInfo( const char *pElementName );
	virtual ~CHudMultiplayerBasicInfo(); // Line 36

	virtual void Init();
	virtual void Reset();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual bool ShouldDraw();
	virtual void Paint();

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
};

DECLARE_HUDELEMENT_DEPTH( CHudMultiplayerBasicInfo, 100 );

CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo( const char *pElementName ) : BaseClass( NULL, "MultiplayerBasicInfo" ), CHudElement( pElementName ) // Line 61
{
	SetParent( GetClientMode()->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	LoadControlSettings( "resource/multiplayerbasicinfo.res" );
}

CHudMultiplayerBasicInfo::~CHudMultiplayerBasicInfo() // Line 36
{
}

void CHudMultiplayerBasicInfo::Init() // Line 75
{
	int w = 0, h = 0;
	GetHudSize( w, h );

	SetSize( 150, 80 );
	SetPos( w - 200, h - 130 );
}

void CHudMultiplayerBasicInfo::Reset() // Line 86
{
	// ?
}

void CHudMultiplayerBasicInfo::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 94
{
	LoadControlSettings( "resource/multiplayerbasicinfo.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( true );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundType( 2 );
	SetVisible( false );
}

bool CHudMultiplayerBasicInfo::ShouldDraw() // Line 109
{
	return PortalMPGameRules() && PortalMPGameRules()->IsMultiplayer() && !PortalMPGameRules()->IsCoOp();
}

void CHudMultiplayerBasicInfo::Paint() // Line 117
{
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();
	if( !pLocalPlayer || !pLocalPlayer->GetTeam() || !g_PR )
		return;

	Color clrEnemyTeam = Color( 0, 0, 0, 128 );
	if( pLocalPlayer->GetTeam()->GetTeamNumber() == TEAM_RED )
	{
		clrEnemyTeam = Color( 255, 75, 20, 128 );
	}
	else if( pLocalPlayer->GetTeam()->GetTeamNumber() == TEAM_BLUE )
	{
		clrEnemyTeam = Color( 100, 175, 255, 128 );
	}

	SetBgColor( clrEnemyTeam );

	wchar_t text[32];
	V_swprintf_safe( text, L"Your Score: %i", g_PR->GetFrags( pLocalPlayer->entindex() ) ); // TODO: GetPlayerScore
	vgui::surface()->DrawSetTextFont( m_hTextFont );
	vgui::surface()->DrawSetTextColor( Color( 0, 0, 0, 255 ) );
	vgui::surface()->DrawSetTextPos( 10, 10 );
	vgui::surface()->DrawUnicodeString( text, FONT_DRAW_DEFAULT );

	int nFontTall = vgui::surface()->GetFontTall( m_hTextFont ) + 20;
	V_swprintf_safe( text, L"Team Score: %i", g_PR->GetTeamScore( pLocalPlayer->GetTeamNumber() ) );
	vgui::surface()->DrawSetTextFont( m_hTextFont );
	vgui::surface()->DrawSetTextColor( Color( 0, 0, 0, 255 ) );
	vgui::surface()->DrawSetTextPos( 10, nFontTall );
	vgui::surface()->DrawUnicodeString( text, FONT_DRAW_DEFAULT );

	int nOtherTeam = TEAM_SPECTATOR;
	clrEnemyTeam = Color( 0, 0, 0, 255 );
	if( pLocalPlayer->GetTeamNumber() == TEAM_BLUE )
	{
		clrEnemyTeam = Color( 255, 0, 0, 255 );
		nOtherTeam = g_PR->GetTeamScore( TEAM_RED );
	}
	else
	{
		clrEnemyTeam = Color( 0, 0, 255, 255 );
		nOtherTeam = g_PR->GetTeamScore( TEAM_BLUE );
	}

	V_swprintf_safe( text, L"Other Team: %i", g_PR->GetTeamScore( nOtherTeam ) );
	vgui::surface()->DrawSetTextFont( m_hTextFont );
	vgui::surface()->DrawSetTextColor( clrEnemyTeam );
	vgui::surface()->DrawSetTextPos( 10, ( nFontTall + vgui::surface()->GetFontTall( m_hTextFont ) + 10 ) );
	vgui::surface()->DrawUnicodeString( text, FONT_DRAW_DEFAULT );
	vgui::surface()->GetFontTall( m_hTextFont ); // ????

	BaseClass::Paint();
}
