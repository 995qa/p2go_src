#if !defined( HUD_TAUNT_EARNED_H )
#define HUD_TAUNT_EARNED_H

#if defined( WIN32 )
#pragma once
#endif

struct TauntStatusData {
	char gap[16];
}; // TODO

class vgui::Label;
class vgui::ImagePanel;

class CHUDTauntEarned : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHUDTauntEarned, vgui::EditablePanel );

public:
	CHUDTauntEarned( const char *pElementName );
	virtual ~CHUDTauntEarned(); // Line 17

	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PaintBackground();
	virtual void Paint();

	void OnTauntUnlocked( const TauntStatusData *pTaunt );

private:
	vgui::Label *m_pGestureLabel;
	vgui::ImagePanel *m_pGestureImage;
	Color m_rgbaCurrent;
};

#endif
