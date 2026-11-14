// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "VisualAndAudio/PWMontageSetComponent.h"

#include "Net/UnrealNetwork.h"
#include "VisualAndAudio/PWMontageSet.h"

UPWMontageSetComponent::UPWMontageSetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPWMontageSetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPWMontageSetComponent, MontageSet);
}

void UPWMontageSetComponent::SetMontageSet(UPWMontageSet* NewSet)
{
	MontageSet = NewSet;
}

UAnimMontage* UPWMontageSetComponent::GetMontageForTag_Implementation(const FGameplayTag AnimTag, float& OutAnimRate) const
{
	OutAnimRate = 1.f;
	return MontageSet ? MontageSet->FindMontage(AnimTag, OutAnimRate) : nullptr;
}
