// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"

#include "Ability/PWSpellParams.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"

float UPWAbilityModule_Radius::GetRadius() const
{
	return GetRadiusAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Radius::GetBaseRadius() const
{
	return GetBaseRadiusAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Radius::GetRadiusAtLevel(const int32 Level) const
{
	return CurrentRadius.Get(Radius, Level);
}

float UPWAbilityModule_Radius::GetBaseRadiusAtLevel(const int32 Level) const
{
	return Radius.GetValueAtLevel(Level);
}

void UPWAbilityModule_Radius::SetRadius(const float NewRadius)
{
	SetRadiusCurve(FScalableFloat(NewRadius));
}

void UPWAbilityModule_Radius::SetRadiusCurve(const FScalableFloat& NewRadius)
{
	CurrentRadius.Set(NewRadius);
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("radius set to %.0f for this cast (configured %.0f)"), GetRadius(), GetBaseRadius()));
}

void UPWAbilityModule_Radius::ResetCurrentValues()
{
	Super::ResetCurrentValues();
	CurrentRadius.Reset();
}

void UPWAbilityModule_Radius::DebugDrawAt(const FVector& Center, const FString& UsedFor, const float Duration) const
{
	if (!IsDebugging())
		return;
	const float Value = GetRadius();
	DebugSphere(Center, Value, PWModuleDebug::Radius, Duration);
	DebugCircle(Center, Value, PWModuleDebug::Radius, Duration);
	DebugText(Center + FVector(0.f, 0.f, Value), FString::Printf(TEXT("Radius %.0f (%s)"), Value, *UsedFor), PWModuleDebug::Radius, Duration);
}

void UPWAbilityModule_Radius::OnAbilityActivated_Implementation()
{
	Super::OnAbilityActivated_Implementation();
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("radius %.0f at level %d"), GetRadius(), GetOwnerLevel()));
}

void UPWAbilityModule_Radius::ExtendParams(UPWSpellParams* Params) const
{
	if (!Params) return;

	Params->More->AddFloat(PWParamKeys::Radius.ToString(), GetRadius());
}

bool UPWAbilityModule_Radius::GetValue(const FName Key, float& Out) const
{
	if (Key != PWParamKeys::Radius)
		return false;

	Out = GetRadius();
	return true;
}
