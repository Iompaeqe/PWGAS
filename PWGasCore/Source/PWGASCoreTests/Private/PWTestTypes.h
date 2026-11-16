// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Ability/PWSpellParamsInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GAS/Abilities/PWAbilityRegistryProvider.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Confirmed.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Instant.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Channeling.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Charged.h"
#include "GAS/Abilities/Modules/DataModules/PWDataModule.h"
#include "PWAttributeSetBase.h"
#include "Core/PWTeamsSubsystem.h"
#include "Agents/PWTeamAgent.h"
#include "Agents/PWTeamAgentComponent.h"
#include "Relationships/PWTeamRelationshipProvider.h"
#include "PWTestTypes.generated.h"

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPlainAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestPlainAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestCostEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestCostAbility : public UPWGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPWTestCostAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestModularAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestModularAbility();

	// A number variable for the Blueprint Variable value to read.
	UPROPERTY()
	int32 KillCount = 0;
};

UCLASS(NotBlueprintable, NotPlaceable)
class APWTestSpellActor : public AActor, public IPWSpellParamsInterface
{
	GENERATED_BODY()

public:
	APWTestSpellActor();

	UPROPERTY()
	TObjectPtr<UPWSpellParams> SpellParams;

	virtual void SetSpellParams_Implementation(UPWSpellParams* Params) override { SpellParams = Params; }
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPassiveEffectAbility : public UPWGameplayAbility_Passive
{
	GENERATED_BODY()

public:
	UPWTestPassiveEffectAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestDurationEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestDurationEffect();
};

// Keeps the default cooldown effect.
UCLASS(NotBlueprintable, HideDropdown)
class UPWTestCooldownAbility : public UPWTestModularAbility
{
	GENERATED_BODY()

public:
	void SetCooldown(const float Seconds) { Cooldown = FScalableFloat(Seconds); }
	void SetAbilityTag(const FGameplayTag Tag) { AbilityTag = Tag; }
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantBuffAbility : public UPWGameplayAbility_Instant
{
	GENERATED_BODY()

public:
	UPWTestInstantBuffAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestTargetResolver : public UPWTargetResolver
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;

	mutable FVector LastLocation = FVector::ZeroVector;
	mutable float LastRadius = 0.f;

	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override
	{
		LastLocation = Target.Location;
		LastRadius = GetFloat(PWParamKeys::Radius);
		OutActors.Append(Actors);
	}
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantEffectAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestInstantEffectAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestConfirmedAbility : public UPWGameplayAbility_Confirmed
{
	GENERATED_BODY()

public:
	UPWTestConfirmedAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestChargedAbility : public UPWGameplayAbility_Charged
{
	GENERATED_BODY()

public:
	UPWTestChargedAbility();

	void SetTickAtStart(const bool bTick) { bTickAtStart = bTick; }
	void SetAutoEndAbility(const bool bAuto) { bAutoEndAbility = bAuto; }
	bool IsCharging() const { return bIsCharging; }
	// Ticks the way the charge timer does, without waiting on it.
	void RunTicks(const int32 Count) { for (int32 i = 0; i < Count; ++i) HandleChargeTick(); }
	// Ends the way a graph's End Ability node does.
	void EndFromGraph() { K2_EndAbility(); }
	// What the server's ability system does once the predicting client's copy of the cast has ended.
	void EndAsRemoteEnded() { SetRemoteInstanceHasEnded(); EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false); }
};

// Records the charge and channel moments it hears about.
UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPhaseModule : public UPWDataModule
{
	GENERATED_BODY()

public:
	TArray<EPWAbilityPhase> Phases;
	// Runs as Completed arrives, where a graph's On Channel End or On Charge Complete would call End Ability.
	TFunction<void()> OnCompleted;

	virtual void ExtendParams(UPWSpellParams* Params) const override {}
	virtual void OnAbilityPhase_Implementation(const EPWAbilityPhase Phase) override
	{
		Phases.Add(Phase);
		if (Phase == EPWAbilityPhase::Completed && OnCompleted)
			OnCompleted();
	}
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestChannelingAbility : public UPWGameplayAbility_Channeling
{
	GENERATED_BODY()

public:
	UPWTestChannelingAbility();

	void SetTickAtStart(const bool bTick) { bTickAtStart = bTick; }
	void SetAutoEndAbility(const bool bAuto) { bAutoEndAbility = bAuto; }
	float GetChannelTime() const { return CurrentChannelTime; }
	bool IsChanneling() const { return bIsChanneling; }
	// Ticks the way the channel timer does, without waiting on it.
	void RunTicks(const int32 Count) { for (int32 i = 0; i < Count; ++i) HandleChannelTick(); }
	// Ends the way a graph's End Ability node does.
	void EndFromGraph() { K2_EndAbility(); }
	// What the server's ability system does once the predicting client's copy of the cast has ended.
	void EndAsRemoteEnded() { SetRemoteInstanceHasEnded(); EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false); }
};

UCLASS(NotBlueprintable)
class UPWTestAttributeSet : public UPWAttributeSetBase
{
	GENERATED_BODY()

public:
	UPWTestAttributeSet();

	UPROPERTY(ReplicatedUsing = OnRep_Health) FGameplayAttributeData Health; UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue); } ATTRIBUTE_ACCESSORS(ThisClass, Health);
	UPROPERTY(ReplicatedUsing = OnRep_HealthMax) FGameplayAttributeData HealthMax; UFUNCTION() void OnRep_HealthMax(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, HealthMax, OldValue); } ATTRIBUTE_ACCESSORS(ThisClass, HealthMax);
	UPROPERTY() FGameplayAttributeData Cooldown; ATTRIBUTE_ACCESSORS(ThisClass, Cooldown);
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPeriodicDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestPeriodicDamageEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestAuraAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestAuraAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantHitEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestInstantHitEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestTimedHitEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestTimedHitEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestLeveledAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestLeveledAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInterruptibleAbility : public UPWGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPWTestInterruptibleAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestTeamsListener : public UObject
{
	GENERATED_BODY()

public:
	int32 Registered = 0;
	int32 Unregistered = 0;
	int32 FactionsCreated = 0;
	int32 FactionsDestroyed = 0;
	int32 FactionChanges = 0;
	FName LastOldFaction;
	FName LastNewFaction;

	UFUNCTION()
	void OnRegistered(const FPWTeamParticipant& Participant) { ++Registered; }
	UFUNCTION()
	void OnUnregistered(const FPWTeamParticipant& Participant) { ++Unregistered; }
	UFUNCTION()
	void OnFactionCreated(const FPWTeamFaction& Faction) { ++FactionsCreated; }
	UFUNCTION()
	void OnFactionDestroyed(const FPWTeamFaction& Faction) { ++FactionsDestroyed; }
	UFUNCTION()
	void OnFactionChanged(FPWTeamParticipantHandle Participant, FName OldFaction, FName NewFaction)
	{
		++FactionChanges;
		LastOldFaction = OldFaction;
		LastNewFaction = NewFaction;
	}

	int32 AgentOwnerChanges = 0;
	int32 AgentFactionChanges = 0;
	FName LastAgentFaction;

	UFUNCTION()
	void OnAgentOwnerChanged(UPWTeamAgentComponent* Agent, FPWTeamParticipantHandle OldOwner, FPWTeamParticipantHandle NewOwner) { ++AgentOwnerChanges; }
	UFUNCTION()
	void OnAgentFactionChanged(UPWTeamAgentComponent* Agent, FName OldFaction, FName NewFaction)
	{
		++AgentFactionChanges;
		LastAgentFaction = NewFaction;
	}
};

// Stands in for a character whose identity lives on another actor, such as its player state.
UCLASS(NotBlueprintable, HideDropdown)
class APWTestAgentProxy : public AActor, public IPWTeamAgent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UPWTeamAgentComponent> Agent;

	virtual UPWTeamAgentComponent* GetTeamAgent_Implementation() const override { return Agent; }
};

// A game's own layer above factions: factions in the same bloc are friendly.
UCLASS(NotBlueprintable, HideDropdown)
class UPWTestBlocProvider : public UPWTeamRelationshipProvider
{
	GENERATED_BODY()

public:
	TMap<FName, FName> BlocOfFaction;
	FGameplayTag SameBlocTag;

	virtual void ResolveRelationship_Implementation(const FPWTeamIdentity& Source, const FPWTeamIdentity& Target, FPWTeamRelationship& Relationship) const override
	{
		const FName* SourceBloc = BlocOfFaction.Find(Source.Faction);
		const FName* TargetBloc = BlocOfFaction.Find(Target.Faction);
		if (!SourceBloc || !TargetBloc || *SourceBloc != *TargetBloc)
			return;

		Relationship.Attitude = EPWTeamAttitude::Friendly;
		Relationship.Tags.AddTag(SameBlocTag);
	}
};

// A rule with nothing to configure: every two factions are friends.
UCLASS(NotBlueprintable, HideDropdown)
class UPWTestFriendlyProvider : public UPWTeamRelationshipProvider
{
	GENERATED_BODY()

public:
	virtual void ResolveRelationship_Implementation(const FPWTeamIdentity& Source, const FPWTeamIdentity& Target, FPWTeamRelationship& Relationship) const override
	{
		if (!Source.Faction.IsNone() && !Target.Faction.IsNone())
			Relationship.Attitude = EPWTeamAttitude::Friendly;
	}
};

// Counts Blueprint events, which need a UFUNCTION to bind to.
UCLASS(HideDropdown)
class UPWTestTagListener : public UObject
{
	GENERATED_BODY()

public:
	int32 Added = 0;
	int32 Removed = 0;

	UFUNCTION()
	void OnAdded(FGameplayTag Tag) { ++Added; }

	UFUNCTION()
	void OnRemoved(FGameplayTag Tag) { ++Removed; }
};

// A game mode that brings its own ability registry.
UCLASS(NotBlueprintable, HideDropdown)
class APWTestRegistryGameMode : public AGameModeBase, public IPWAbilityRegistryProvider
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UPWAbilityRegistry> Registry;

	virtual UPWAbilityRegistry* GetAbilityRegistry_Implementation() const override { return Registry; }
};
