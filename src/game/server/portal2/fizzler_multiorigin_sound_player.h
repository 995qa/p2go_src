#if !defined( FIZZLER_MULTIORIGIN_SOUND_PLAYER_H )
#define FIZZLER_MULTIORIGIN_SOUND_PLAYER_H

#if defined( WIN32 )
#pragma once
#endif

class FizzlerMultiOriginSoundPlayer : public CBaseEntity
{
	DECLARE_CLASS( FizzlerMultiOriginSoundPlayer, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	FizzlerMultiOriginSoundPlayer();
	virtual ~FizzlerMultiOriginSoundPlayer();

	virtual void Spawn();
	void RemoveThink();

	virtual int UpdateTransmitState( void );
	
	// FIXME(SanyaSho): Should this be static?
	static FizzlerMultiOriginSoundPlayer *Create( IRecipientFilter &filter, const char *soundName );
	
private:
	CSoundPatch *m_pSound;
};

#endif