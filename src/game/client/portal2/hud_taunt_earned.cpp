#include "cbase.h"
#include "vgui_controls/EditablePanel.h"
#include "hudelement.h"
#include "hud.h"
#include "hud_taunt_earned.h"
#include "clientmode_shared.h"
#include "vgui_controls/ImagePanel.h"

// gay
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHUDTauntEarned );

CHUDTauntEarned::CHUDTauntEarned( const char *pElementName ) : BaseClass( NULL, "HudTauntEarned" ), CHudElement( pElementName ) // Line 24
{
	SetParent( GetClientMode()->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_pGestureLabel = new vgui::Label( this, "GrestureLabel", L"" );
	m_pGestureImage = new vgui::ImagePanel( this, "GrestureImage" );
}

CHUDTauntEarned::~CHUDTauntEarned() // Line 17
{
}



void CHUDTauntEarned::PerformLayout() // Line 42
{
	BaseClass::PerformLayout();
	SetAlpha( 0 );

	//( ( void( __thiscall * )( CHUDTauntEarned *, _DWORD ) )this->VidInit )( this, 0 ); // INCORRECT!
	SetVisible( false );
}

void CHUDTauntEarned::ApplySchemeSettings( vgui::IScheme *pScheme ) // Line 35
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/ui/hud_taunt_earned.res" );
}

void CHUDTauntEarned::OnTauntUnlocked( const TauntStatusData *pTaunt ) // Line 50
{
	// TODO: Debug-only stuff?
}

void CHUDTauntEarned::PaintBackground() // Line 76
{
	// TODO:
}

void CHUDTauntEarned::Paint() // Line 137
{
	BaseClass::Paint();
}