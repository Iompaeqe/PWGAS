// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "World/PWDamageableComponent.h"

#include "Ability/SpellParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Libraries/GASCoreFunctions.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UPWDamageableComponent::UPWDamageableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UPWDamageableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPWDamageableComponent, Health);
}

void UPWDamageableComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (HasAuthority())
		Health = MaxHealth;
}

void UPWDamageableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearOngoingEffects();
	Super::EndPlay(EndPlayReason);
}

bool UPWDamageableComponent::ApplySpellParams(USpellParams* SpellParams)
{
	if (!SpellParams || !HasAuthority() || bDepleted)
		return false;

	UWorld* World = GetWorld();
	AActor* Instigator = SpellParams->SourceAvatar;
	const TArray<FEffectModifierValue> Values = UGASCoreFunctions::GetModifierValuesFromEffects(SpellParams, DamageAttribute, GetOwner());

	for (const FEffectModifierValue& Value : Values)
	{
		if (Value.DurationType == EGameplayEffectDurationType::Instant)
		{
			ApplyDamage(Value.Value, Instigator);
			continue;
		}
		if (Value.Period <= 0.f || !World)
			continue;

		// The same effect from the same caster restarts, it does not stack.
		OngoingEffects.RemoveAll([&](const FOngoingEffect& Existing)
		{
			if (Existing.Instigator != Instigator || Existing.EffectClass != Value.EffectClass)
				return false;
			StopOngoing(Existing);
			return true;
		});

		FOngoingEffect& Ongoing = OngoingEffects.AddDefaulted_GetRef();
		Ongoing.Instigator = Instigator;
		Ongoing.EffectClass = Value.EffectClass;
		Ongoing.bInfinite = Value.DurationType == EGameplayEffectDurationType::Infinite;
		World->GetTimerManager().SetTimer(Ongoing.Tick, FTimerDelegate::CreateUObject(this, &ThisClass::TickOngoing, Value.Value, TWeakObjectPtr<AActor>(Instigator)), Value.Period, true);

		if (!Ongoing.bInfinite && Value.Duration > 0.f)
			World->GetTimerManager().SetTimer(Ongoing.End, FTimerDelegate::CreateUObject(this, &ThisClass::EndOngoing, Ongoing.Tick), Value.Duration, false);
	}
	return Values.Num() > 0;
}

void UPWDamageableComponent::ApplyDamage(const float Amount, AActor* Instigator)
{
	if (!HasAuthority() || bDepleted)
		return;

	const float Final = ModifyDamage(Amount, Instigator);
	const float OldHealth = Health;
	Health = FMath::Clamp(Health - Final, 0.f, MaxHealth);
	if (Health == OldHealth)
		return;

	OnHealthChanged.Broadcast(this, OldHealth, Health, Instigator);
	if (Health > 0.f)
		return;

	bDepleted = true;
	ClearOngoingEffects();
	OnDepleted.Broadcast(this, Instigator);
	HandleDepleted(Instigator);
}

void UPWDamageableComponent::RemoveSpellParams(const USpellParams* SpellParams, const bool bInfiniteOnly)
{
	if (!SpellParams)
		return;

	const AActor* Instigator = SpellParams->SourceAvatar;
	TArray<TSubclassOf<UGameplayEffect>> Classes;
	for (const FEffectModifierValue& Value : UGASCoreFunctions::GetModifierValuesFromEffects(SpellParams, DamageAttribute, GetOwner()))
		Classes.Add(Value.EffectClass);

	OngoingEffects.RemoveAll([&](const FOngoingEffect& Existing)
	{
		if (Existing.Instigator != Instigator || !Classes.Contains(Existing.EffectClass) || (bInfiniteOnly && !Existing.bInfinite))
			return false;
		StopOngoing(Existing);
		return true;
	});
}

void UPWDamageableComponent::ClearOngoingEffects()
{
	for (const FOngoingEffect& Effect : OngoingEffects)
		StopOngoing(Effect);
	OngoingEffects.Reset();
}

void UPWDamageableComponent::StopOngoing(const FOngoingEffect& Effect) const
{
	if (const UWorld* World = GetWorld())
	{
		FTimerHandle Tick = Effect.Tick;
		FTimerHandle End = Effect.End;
		World->GetTimerManager().ClearTimer(Tick);
		World->GetTimerManager().ClearTimer(End);
	}
}

void UPWDamageableComponent::ResetHealth()
{
	if (!HasAuthority())
		return;

	ClearOngoingEffects();
	bDepleted = false;
	const float OldHealth = Health;
	Health = MaxHealth;
	if (Health != OldHealth)
		OnHealthChanged.Broadcast(this, OldHealth, Health, nullptr);
}

float UPWDamageableComponent::ModifyDamage_Implementation(const float Amount, AActor* Instigator) const
{
	return Amount;
}

void UPWDamageableComponent::HandleDepleted_Implementation(AActor* Instigator)
{
	if (bDestroyOwnerWhenDepleted && GetOwner())
		GetOwner()->Destroy();
}

void UPWDamageableComponent::OnRep_Health(const float OldHealth)
{
	OnHealthChanged.Broadcast(this, OldHealth, Health, nullptr);
}

void UPWDamageableComponent::TickOngoing(const float Amount, const TWeakObjectPtr<AActor> Instigator)
{
	ApplyDamage(Amount, Instigator.Get());
}

void UPWDamageableComponent::EndOngoing(FTimerHandle TickHandle)
{
	OngoingEffects.RemoveAll([&](const FOngoingEffect& Existing)
	{
		if (Existing.Tick != TickHandle)
			return false;
		StopOngoing(Existing);
		return true;
	});
}

bool UPWDamageableComponent::HasAuthority() const
{
	return GetOwner() && GetOwner()->HasAuthority();
}
