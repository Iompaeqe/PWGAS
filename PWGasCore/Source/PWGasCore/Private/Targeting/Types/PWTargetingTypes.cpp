// copyright mkurtt96

#include "Targeting/Types/PWTargetPolicyBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Targeting/Data/PWTargetingData.h"

void UPWTargetPolicyBase::Initialize_Implementation(UPWModularGameplayAbility* InOwner)
{
	OwnerAbility = InOwner;
}

void UPWTargetPolicyBase::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
}

bool UPWTargetingSource::Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const
{
	return false;
}

bool UPWTargetingSource::SampleFrom_Implementation(const APawn* Pawn, const FVector& Origin, FPWTargetingResult& Out) const
{
	Out.Location = Origin;
	return true;
}

void UPWRangePolicy::Apply_Implementation(const AActor* Actor, float MaxRange, FPWTargetingResult& InOut) const
{
}

void UPWTargetResolver::Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const
{
}

