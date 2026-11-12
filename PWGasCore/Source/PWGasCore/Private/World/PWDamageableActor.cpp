// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "World/PWDamageableActor.h"

#include "World/PWDamageableComponent.h"

APWDamageableActor::APWDamageableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Damageable = CreateDefaultSubobject<UPWDamageableComponent>("Damageable");
}

bool APWDamageableActor::ApplyEffects_Implementation(USpellParams* SpellParams)
{
	return Damageable->ApplySpellParams(SpellParams);
}

void APWDamageableActor::RemoveEffects_Implementation(USpellParams* SpellParams, const bool bInfiniteOnly)
{
	Damageable->RemoveSpellParams(SpellParams, bInfiniteOnly);
}

UObject* APWDamageableActor::GetEffectReceiver_Implementation()
{
	return this;
}
