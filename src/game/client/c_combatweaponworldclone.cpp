#include "cbase.h"

#include "c_baseanimating.h"

#include "c_combatweaponworldclone.h"









C_CombatWeaponClone::C_CombatWeaponClone( C_BaseCombatWeapon *pWeaponParent )
{
	m_pWeaponParent = pWeaponParent;
	InitializeAsClientEntity( pWeaponParent->GetModelName(), false );
}

C_CombatWeaponClone::~C_CombatWeaponClone()
{
	//m_pWeaponParent->NotifyWorldModelCloneReleased();
}


IClientModelRenderable *C_CombatWeaponClone::GetClientModelRenderable()
{
	if( !m_bReadyToDraw || m_pWeaponParent->IsFirstPersonSpectated() /*|| !( *( int(__thiscall **)( int ) )( *(_DWORD *)( *(_DWORD *)( &this->m_bDynamicModelPending + 3 ) + 4 ) + 176 ) )( *(_DWORD *)( &this->m_bDynamicModelPending + 3 ) + 4 )*/ )
	{
		return NULL;
	}

	UpdateClone();
	return BaseClass::GetClientModelRenderable();
}


int C_CombatWeaponClone::DrawModel( int flags, const RenderableInstance_t &instance ) // 39
{
	UpdateClone();
	VPROF( "PORTAL GUN" );
	return BaseClass::DrawModel( flags, instance );
}


bool C_CombatWeaponClone::ShouldSuppressForSplitScreenPlayer( int nSlot ) // 47
{
	C_BaseCombatCharacter *pOwner = m_pWeaponParent->GetOwner();

	return pOwner
		&& pOwner->IsPlayer()
		&& dynamic_cast< C_BasePlayer * >( pOwner )->IsLocalPlayer()
		&& pOwner->ShouldSuppressForSplitScreenPlayer( nSlot );
}







bool C_CombatWeaponClone::ShouldDraw() // 63
{
	C_BaseCombatCharacter *pOwner = m_pWeaponParent->GetOwner();
	/*return ( !Owner || !( *( ( unsigned __int8( __cdecl ** )( C_BaseCombatCharacter * ) )Owner->_vptr_IHandleEntity + 200 ) )( Owner ) || !C_BasePlayer::IsLocalPlayer( v2 ) || ( *( ( unsigned __int8( __cdecl ** )( const C_BaseEntity * ) )v2->_vptr_IHandleEntity + 145 ) )( v2 ) )
		&& C_BaseCombatWeapon::ShouldDrawThisOrWorldModelClone( this->m_pWeaponParent ) && C_BaseCombatWeapon::GetOwner( this->m_pWeaponParent ) && !( *( ( unsigned __int8( __cdecl ** )( C_BaseCombatWeapon * ) )this->m_pWeaponParent->_vptr_IHandleEntity + 79 ) )( this->m_pWeaponParent ) && C_BaseEntity::ShouldDraw( this );*/
	return true;
}






bool C_CombatWeaponClone::SetupBones( matrix3x4a_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime ) // 76
{
	UpdateClone();
	m_nWeaponBoneIndex = -1;
	return BaseClass::SetupBones( pBoneToWorldOut, nMaxBones, boneMask, currentTime );
}







bool C_CombatWeaponClone::ComputeStencilState( ShaderStencilState_t *pStencilState ) // 89
{
	return false; //m_pWeaponParent->ComputeStencilState( pStencilState ); // TODO: protected xbox-only function
}

void C_CombatWeaponClone::UpdateClone() // 94
{
}











































const Vector &C_CombatWeaponClone::GetRenderOrigin() // 140
{
	if( m_nWeaponBoneIndex < 0 )
		return BaseClass::GetRenderOrigin();
	else
		return m_vWeaponBonePosition;
}


const QAngle &C_CombatWeaponClone::GetRenderAngles() // 149
{
	if( m_nWeaponBoneIndex < 0 )
		return BaseClass::C_BaseAnimating::GetRenderAngles();
	else
		return m_qWeaponBoneAngle;
}


C_BaseCombatWeapon *C_CombatWeaponClone::MyCombatWeaponPointer() // 158
{
	return NULL;
}