// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"

#include "Debug/Logging.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

float UPWAbilityModule_Range::GetRange() const
{
	return GetRangeAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Range::GetBaseRange() const
{
	return GetBaseRangeAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Range::GetRangeAtLevel(const int32 Level) const
{
	return CurrentRange.Get(Range, Level);
}

float UPWAbilityModule_Range::GetBaseRangeAtLevel(const int32 Level) const
{
	return Range.GetValueAtLevel(Level);
}

void UPWAbilityModule_Range::SetRange(const float NewRange)
{
	SetRangeCurve(FScalableFloat(NewRange));
}

void UPWAbilityModule_Range::SetRangeCurve(const FScalableFloat& NewRange)
{
	CurrentRange.Set(NewRange);
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("range set to %.0f for this cast (configured %.0f)"), GetRange(), GetBaseRange()));
}

void UPWAbilityModule_Range::ResetCurrentValues()
{
	Super::ResetCurrentValues();
	CurrentRange.Reset();
}

void UPWAbilityModule_Range::ApplyRangePolicy(const AActor* Origin, FPWTargetingResult& InOut) const
{
	if (!RangePolicy)
	{
		pwlogability(Error, "ApplyRangePolicy failed: RangePolicy is null in %s", *GetNameSafe(this));
		return;
	}

	if (!Origin)
	{
		pwlogability(Warning, "ApplyRangePolicy: Origin actor is null for %s", *GetNameSafe(this));
		return;
	}

	if (!OwnerAbility)
	{
		pwlogability(Error, "ApplyRangePolicy failed: OwnerAbility is null for %s", *GetNameSafe(this));
		return;
	}

	RangePolicy->Initialize(OwnerAbility);

	const float RangeValue = GetRange();
	if (RangeValue <= 0.f)
	{
		pwlogability(Warning, "ApplyRangePolicy: Range value <= 0 in %s", *GetNameSafe(this));
		return;
	}

	const FVector Aimed = InOut.Location;
	RangePolicy->Apply(Origin, RangeValue, InOut);

	if (!IsDebugging())
		return;
	const FVector From = Origin->GetActorLocation();
	const bool bMoved = !Aimed.Equals(InOut.Location, 1.f);
	DebugDrawFrom(From, TEXT("cast"));
	DebugArrow(From, InOut.Location, PWModuleDebug::Range);
	if (bMoved)
	{
		DebugPoint(Aimed, PWModuleDebug::Target);
		DebugLine(Aimed, InOut.Location, PWModuleDebug::Target);
	}
	DebugPrint(FString::Printf(TEXT("range %.0f, aimed %.0f away, %s by %s"), RangeValue, FVector::Dist(From, Aimed),
		bMoved ? *FString::Printf(TEXT("moved to %.0f away"), FVector::Dist(From, InOut.Location)) : TEXT("left as is"), *GetNameSafe(RangePolicy)));
}

void UPWAbilityModule_Range::DebugDrawFrom(const FVector& Origin, const FString& UsedFor, const float Duration) const
{
	if (!IsDebugging())
		return;
	const float Value = GetRange();
	DebugCircle(Origin, Value, PWModuleDebug::Range, Duration);
	DebugText(Origin + FVector(0.f, 0.f, 120.f), FString::Printf(TEXT("Range %.0f (%s)"), Value, *UsedFor), PWModuleDebug::Range, Duration);
}

void UPWAbilityModule_Range::OnAbilityActivated_Implementation()
{
	Super::OnAbilityActivated_Implementation();
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("range %.0f at level %d, policy %s"), GetRange(), OwnerAbility->GetAbilityLevel(), *GetNameSafe(RangePolicy)));
}

void UPWAbilityModule_Range::Initialize(UPWModularGameplayAbility* InOwner)
{
	Super::Initialize(InOwner);
	if (RangePolicy) 
		RangePolicy->Initialize(InOwner);
}

void UPWAbilityModule_Range::ExtendParams(UPWSpellParams* Params) const
{
	if (!Params)
		return;

	Params->More->AddFloat(PWParamKeys::Range.ToString(), GetRange());
}

bool UPWAbilityModule_Range::GetValue(const FName Key, float& Out) const
{
	if (Key != PWParamKeys::Range)
		return false;

	Out = GetRange();
	return true;
}
