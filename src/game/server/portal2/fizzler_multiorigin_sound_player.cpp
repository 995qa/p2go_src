#include "cbase.h"
#include "portal_mp_gamerules.h"
#include "trigger_portal_cleanser.h"
#include "fizzler_multiorigin_sound_player.h"
#include "soundenvelope.h"

// gay
#include "tier0/memdbgon.h"

BEGIN_DATADESC( FizzlerMultiOriginSoundPlayer )
	DEFINE_SOUNDPATCH( m_pSound ),
	DEFINE_THINKFUNC( RemoveThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( FizzlerMultiOriginSoundPlayer, DT_FizzlerMultiOriginSoundPlayer )
	//
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( fizzler_multiorigin_sound_player, FizzlerMultiOriginSoundPlayer );

FizzlerMultiOriginSoundPlayer::FizzlerMultiOriginSoundPlayer() // Line 28
{
	m_pSound = NULL;
}

FizzlerMultiOriginSoundPlayer::~FizzlerMultiOriginSoundPlayer() // Line 33
{
	if( m_pSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.Shutdown( m_pSound );
		controller.SoundDestroy( m_pSound );
	}
}

void FizzlerMultiOriginSoundPlayer::Spawn() // Line 58
{
	BaseClass::Spawn();
	SetThink( &FizzlerMultiOriginSoundPlayer::RemoveThink );
	SetNextThink( gpGlobals->curtime );
}

FizzlerMultiOriginSoundPlayer *FizzlerMultiOriginSoundPlayer::Create( IRecipientFilter &filter, const char *soundName ) // Line 44
{
	FizzlerMultiOriginSoundPlayer *pFizzlerSoundPlayer = dynamic_cast<FizzlerMultiOriginSoundPlayer *>( CreateEntityByName( "fizzler_multiorigin_sound_player" ) );
	if( pFizzlerSoundPlayer )
	{
		pFizzlerSoundPlayer->Spawn();

		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		pFizzlerSoundPlayer->m_pSound = controller.SoundCreate( filter, pFizzlerSoundPlayer->entindex(), soundName );
		controller.Play( pFizzlerSoundPlayer->m_pSound, 1.0, PITCH_NORM );
	}

	return pFizzlerSoundPlayer;
}

void FizzlerMultiOriginSoundPlayer::RemoveThink() // Line 71
{
	if( ITriggerPortalCleanserAutoList::AutoList().Count() ) // is someone there?
		SetNextThink( gpGlobals->curtime );

	UTIL_Remove( this );
}

int FizzlerMultiOriginSoundPlayer::UpdateTransmitState() // Line 66
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}