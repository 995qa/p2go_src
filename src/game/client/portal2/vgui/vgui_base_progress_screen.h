#if !defined( VGUI_BASE_PROGRESS_SCREEN_H )
#define VGUI_BASE_PROGRESS_SCREEN_H

#if defined( WIN32 )
#pragma once
#endif

class CVGuiScreenPanel;
class CAutoGameSystem;

#define DEFAULT_INFO_PANEL_STARTUP_SEQUENCE "default_flicker"

struct ProgressSignStartupSequence_t
{
	ProgressSignStartupSequence_t()
	{
		V_strcpy_safe( m_szName, DEFAULT_INFO_PANEL_STARTUP_SEQUENCE );
		m_flFlickerQuickMin = 0.2f;
		m_flFlickerRateMax = 0.64f;
		m_flFlickerQuickMin = 0.02f;
		m_flFlickerQuickMax = 0.12f;
		m_flBGLength = 0.75;
		m_flNumDelay = 0.8f;
		m_flIconDelay = 1.f;
		m_flProgDelay = 1.65f;
	}

	char m_szName[128];
	float m_flFlickerRateMin;
	float m_flFlickerRateMax;
	float m_flFlickerQuickMin;
	float m_flFlickerQuickMax;
	float m_flBGLength;
	float m_flNumDelay;
	float m_flProgDelay;
	float m_flIconDelay;
};

class CProgressSignScreenKeys : public CAutoGameSystem
{
public:
	CProgressSignScreenKeys( char const *name );

	virtual bool Init();

	bool SetupStartupSequenecs();

	KeyValues *GetSPIconKeyValues();

	KeyValues *GetMPIconKeyValues();

	int GetRandomStartupSequenceIndex();

	int FindStartupSequenceIndex( const char *pszSequenceName );

	void GetStartupSequences( int nIndex, ProgressSignStartupSequence_t *sequence );

private:
	KeyValues *m_pPreloadedSPIconKeys;
	KeyValues *m_pPreloadedMPIconKeys;

	bool m_bStartupSequencesInitiailized;
	CUtlVector< ProgressSignStartupSequence_t > m_StartupSequences;
};

extern CProgressSignScreenKeys g_ProgressSignScreenKeys;



#define NUM_ICONS 10
#define NUM_DIRT_OVERLAYS 3

class CVGUI_Base_ProgressSignScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_Base_ProgressSignScreen, CVGuiScreenPanel );

public:
	CVGUI_Base_ProgressSignScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CVGUI_Base_ProgressSignScreen();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void OnTick();

protected:
	void BeginStartupSequence();
	float UpdateStartupSequence();

	void ResetLevelData();

	void RenderBackground();
	void RenderLevelNumber( int, float );
	void RenderLevelProgress( int, int, float );
	void RenderIcons();
	void RenderDirtOverlays();

	int m_nDisplayedIcons[NUM_ICONS];
	bool m_bHighlightedIcons[NUM_ICONS];

	int m_nStartupSequenceIndex;
	bool m_bStartingUp;

	int m_nSU_Dirt_CurAlpha;
	int m_nMaxBGAlpha;
	float m_flGreyedIconAlpha;
	int m_nDirtOverlayID;

private:
	int                        m_nBackgroundTextureID;
	int                        m_nNumberTextureID;
	int                        m_nProgBarTextureID;
	int                        m_nIconsTextureID;
	float                      m_flSU_StartTime;
	float                      m_flSU_Length;
	int                        m_nSU_BG_CurAlpha;
	float                      m_flSU_IconAlphaScale;
	float                      m_flSU_ProgressLabelAlpha;
	bool                       m_bIsEnabledOld;

	float                      m_flSU_FlickerRate;
	float                      m_flSU_LastFlicker;
	float                      m_flSU_FlickerRateQuick;
	int                        m_nDirtOverlayTextures[NUM_DIRT_OVERLAYS];
	bool                       m_bIsAlreadyVisible;
};

extern const char *g_pszLightboardIcons[];

#endif
