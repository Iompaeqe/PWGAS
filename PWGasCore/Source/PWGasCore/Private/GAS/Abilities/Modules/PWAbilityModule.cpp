// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/PWAbilityModule.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGASCoreLogChannels.h"
#include "UnrealEngine.h"

void UPWAbilityModule::Initialize(UPWModularGameplayAbility* InOwner)
{
	OwnerAbility = InOwner;
}

void UPWAbilityModule::OnAbilityEnded_Implementation()
{
}

void UPWAbilityModule::OnAbilityActivated_Implementation()
{
}

void UPWAbilityModule::OnAbilityPhase_Implementation(EPWAbilityPhase Phase)
{
}

void UPWAbilityModule::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
}

APlayerController* UPWAbilityModule::GetPC() const
{
	return OwnerAbility ? OwnerAbility->GetPC() : nullptr;
}

APawn* UPWAbilityModule::GetPawn() const
{
	return OwnerAbility ? OwnerAbility->GetPawn() : nullptr;
}

UWorld* UPWAbilityModule::GetDebugWorld() const
{
	return OwnerAbility ? OwnerAbility->GetWorld() : nullptr;
}

int32 UPWAbilityModule::GetOwnerLevel() const
{
	return OwnerAbility ? OwnerAbility->GetAbilityLevel() : 1;
}

bool UPWAbilityModule::IsDebugging() const
{
#if ENABLE_DRAW_DEBUG
	return bShowDebug && GetDebugWorld();
#else
	return false;
#endif
}

int32 UPWAbilityModule::DebugKey(const int32 Slot) const
{
	return static_cast<int32>(HashCombine(GetTypeHash(this), GetTypeHash(Slot)) & 0x7FFFFFFF);
}

void UPWAbilityModule::DebugPrint(const FString& Message, const FLinearColor Color, const float Duration, const int32 Key) const
{
#if ENABLE_DRAW_DEBUG
	if (!IsDebugging())
		return;

	FString ModuleName = GetClass()->GetName();
	ModuleName.RemoveFromStart(TEXT("PWAbilityModule_"));
	ModuleName.RemoveFromEnd(TEXT("_C"));
	FString AbilityName = OwnerAbility->GetClass()->GetName();
	AbilityName.RemoveFromEnd(TEXT("_C"));

	const FString Line = FString::Printf(TEXT("[%s] %s / %s: %s"), *GetDebugStringForWorld(GetDebugWorld()), *AbilityName, *ModuleName, *Message);
	if (Key == INDEX_NONE)
		UE_LOG(LogPWGASCoreAbilityModule, Display, TEXT("%s"), *Line);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(Key, Duration, Color.ToFColor(true), Line);
#endif
}

void UPWAbilityModule::DebugSphere(const FVector& Center, const float Radius, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugSphere(GetDebugWorld(), Center, Radius, 32, Color, false, Duration, 0, 1.5f);
#endif
}

void UPWAbilityModule::DebugCircle(const FVector& Center, const float Radius, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugCircle(GetDebugWorld(), Center, Radius, 64, Color, false, Duration, 0, 3.f, FVector::ForwardVector, FVector::RightVector, false);
#endif
}

void UPWAbilityModule::DebugLine(const FVector& From, const FVector& To, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugLine(GetDebugWorld(), From, To, Color, false, Duration, 0, 2.f);
#endif
}

void UPWAbilityModule::DebugArrow(const FVector& From, const FVector& To, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugDirectionalArrow(GetDebugWorld(), From, To, FMath::Clamp(FVector::Dist(From, To) * 0.2f, 15.f, 60.f), Color, false, Duration, 0, 2.5f);
#endif
}

void UPWAbilityModule::DebugPoint(const FVector& Location, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugPoint(GetDebugWorld(), Location, 14.f, Color, false, Duration);
#endif
}

void UPWAbilityModule::DebugAxes(const FTransform& Transform, const float Size, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugCoordinateSystem(GetDebugWorld(), Transform.GetLocation(), Transform.Rotator(), Size, false, Duration, 0, 2.f);
#endif
}

void UPWAbilityModule::DebugText(const FVector& Location, const FString& Text, const FColor& Color, const float Duration) const
{
#if ENABLE_DRAW_DEBUG
	if (IsDebugging())
		DrawDebugString(GetDebugWorld(), Location, Text, nullptr, Color, Duration, true, 1.2f);
#endif
}
