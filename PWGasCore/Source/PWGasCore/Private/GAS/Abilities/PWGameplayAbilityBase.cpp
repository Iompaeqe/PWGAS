// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/PWGameplayAbilityBase.h"

#include "Core/PWTeamsSubsystem.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayCueManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "PWGASCoreLogChannels.h"
#include "Ability/PWSpellParams.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Effects/PWGameplayEffect_DynamicCooldown.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Libraries/PWGasFunctions.h"
#include "VisualAndAudio/PWMontageProvider.h"



#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue) \
{ \
if (!ensure(IsInstantiated())) \
{ \
ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName()); \
return ReturnValue; \
} \
}

UPWGameplayAbilityBase::UPWGameplayAbilityBase()
{
	ActivationGroup = EPWActivationGroup::Independent;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = UPWGameplayEffect_DynamicCooldown::StaticClass();
}

float UPWGameplayAbilityBase::GetCooldown(int32 Level) const
{
	return Cooldown.GetValueAtLevel(Level);
}

void UPWGameplayAbilityBase::GetCost(int32 Level, TArray<FPWCostData>& CostData) const
{
	UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (!CostGE) return;

	for (FGameplayModifierInfo Modifier : CostGE->Modifiers)
	{
		float Value = 0.f;
		Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(Level, Value);
		FPWCostData Data = FPWCostData(Modifier.Attribute.AttributeName, Modifier.ModifierOp, Value);
		CostData.Add(Data);
	}
}

void UPWGameplayAbilityBase::GetAnimMontageFromActor(UAnimMontage*& OutMontage, float& OutAnimRate, int Index) const
{
	if (AnimationTags.IsValidIndex(Index))
	{
		GetAnimMontageForTag(AnimationTags[Index], OutMontage, OutAnimRate);
		return;
	}
	OutMontage = nullptr;
	OutAnimRate = 1.f;
}

void UPWGameplayAbilityBase::GetAnimMontageForTag(const FGameplayTag AnimTag, UAnimMontage*& OutMontage, float& OutAnimRate) const
{
	OutMontage = nullptr;
	OutAnimRate = 1.f;
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AnimTag.IsValid())
		return;

	if (const UObject* Provider = IPWMontageProvider::Find(AvatarActor))
	{
		OutMontage = IPWMontageProvider::Execute_GetMontageForTag(Provider, AnimTag, OutAnimRate);
		return;
	}
	UE_LOG(LogPWGASCoreAbility, Warning, TEXT("%s: %s has no montage provider. Add a PW Montage Set component, or implement PW Montage Provider."), *GetName(), *AvatarActor->GetName())
}

UAnimInstance* UPWGameplayAbilityBase::GetAnimInstanceFromActor() const
{
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	if (UAnimInstance* AnimInstance = Info ? Info->GetAnimInstance() : nullptr)
		return AnimInstance;
	UE_LOG(LogPWGASCoreAbility, Log, TEXT("%s: the avatar has no animated skeletal mesh."), *GetName());
	return nullptr;
}

void UPWGameplayAbilityBase::ExecuteCueAtSocket(const FGameplayTag CueTag, const FName SocketName, const FVector Offset, const FRotator RotationOffset)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Avatar || !ASC) return;

	const UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Avatar, SocketName);
	const FVector Origin = Mesh ? Mesh->GetSocketLocation(SocketName) : Avatar->GetActorLocation();

	FGameplayCueParameters Parameters;
	Parameters.Instigator = Avatar;
	Parameters.Location = Origin + Avatar->GetActorRotation().RotateVector(Offset);
	Parameters.Normal = (Avatar->GetActorQuat() * RotationOffset.Quaternion()).Vector();
	Parameters.AbilityLevel = GetAbilityLevel();

	const FPredictionKey PredictionKey = GetCurrentActivationInfo().GetActivationPredictionKey();
	if (GetCurrentActorInfo()->IsNetAuthority())
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->InvokeGameplayCueExecuted_WithParams(ASC, CueTag, PredictionKey, Parameters);
	else if (PredictionKey.IsLocalClientKey())
		ASC->InvokeGameplayCueEvent(CueTag, EGameplayCueEvent::Executed, Parameters);
}

FGameplayTag UPWGameplayAbilityBase::GetActivationInputTag() const
{
	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
		for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
			if (Tag.MatchesTag(PWTags::Input::Root))
				return Tag;
	return FGameplayTag();
}

bool UPWGameplayAbilityBase::CanChangeActivationGroup(EPWActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	UPWAbilitySystemComponent* ASC = GetPWAbilitySystemComponentFromActorInfo();
	check(ASC);

	if ((ActivationGroup != EPWActivationGroup::Exclusive_Blocking) && ASC->Abilities().IsActivationGroupBlocked(NewGroup))
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((NewGroup == EPWActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}

	return true;
}

bool UPWGameplayAbilityBase::ChangeActivationGroup(EPWActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
	{
		return false;
	}

	if (ActivationGroup != NewGroup)
	{
		UPWAbilitySystemComponent* ASC = GetPWAbilitySystemComponentFromActorInfo();
		check(ASC);

		ASC->Abilities().RemoveAbilityFromActivationGroup(ActivationGroup, this);
		ASC->Abilities().AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

UPWSpellParams* UPWGameplayAbilityBase::MakeSpellParams()
{
	UPWSpellParams* Params = NewObject<UPWSpellParams>(this);
	Params->SourceAvatar = GetAvatarActorFromActorInfo();
	Params->SourceParticipant = UPWTeamsSubsystem::GetAgentOwner(Params->SourceAvatar);
	Params->AbilityLevel = GetAbilityLevel();

	if (const APlayerState* PS = GetPawn() ? GetPawn()->GetPlayerState() : nullptr)
		Params->SourcePlayerUniqueId = PS->GetUniqueId();

	if (!Params->More)
		Params->More = NewObject<UPWMultiDataArray>(Params);

	return Params;
}

APlayerController* UPWGameplayAbilityBase::GetPC() const
{
	if (const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo())
		return Cast<APlayerController>(Info->PlayerController.Get());
	return nullptr;
}

APawn* UPWGameplayAbilityBase::GetPawn() const
{
	if (const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo())
		return Cast<APawn>(Info->AvatarActor.Get());
	return nullptr;
}

void UPWGameplayAbilityBase::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == EPWActivationGroup::Exclusive_Replaceable))
	{
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

bool UPWGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// An interruption tag also stops it from starting.
	if (ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(InterruptionTags))
	{
		if (OptionalRelevantTags)
			OptionalRelevantTags->AddTag(PWTags::Ability::Activation::Failed);
		return false;
	}

	UPWAbilitySystemComponent* ASC = Cast<UPWAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC && ASC->Abilities().IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(PWTags::Ability::Activation::Failed);
		}
		return false;
	}

	return true;
}

const FGameplayTagContainer* UPWGameplayAbilityBase::GetCooldownTags() const
{
	CooldownTagsCache.Reset();

	if (const FGameplayTagContainer* Granted = Super::GetCooldownTags())
		CooldownTagsCache.AppendTags(*Granted);

	if (AbilityTag.IsValid())
	{
		const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
		if (CooldownTag.IsValid())
			CooldownTagsCache.AddTag(CooldownTag);
	}

	return &CooldownTagsCache;
}

void UPWGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	float Cd = Cooldown.GetValueAtLevel(GetAbilityLevel());
	if (const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect())
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		if (!SpecHandle.IsValid())
			return;

		const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTag.GetSingleTagContainer());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(PWTags::Ability::SetByCaller::Cooldown, Cd);

		// A duration effect that works out to 0 seconds is never timed out, so its tag would stay forever. A cooldown of 0 is no cooldown.
		float Duration = 0.f;
		if (CooldownGE->DurationPolicy == EGameplayEffectDurationType::HasDuration
			&& CooldownGE->DurationMagnitude.AttemptCalculateMagnitude(*SpecHandle.Data, Duration, false)
			&& Duration <= 0.f)
			return;

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}

	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	UPWAbilitySystemComponent* ASC = GetPWAbilitySystemComponentFromActorInfo();
	if (Info && ASC && Info->IsLocallyControlled())
		ASC->Cooldown().CooldownStarted.Broadcast(AbilityTag, Cd);
}

void UPWGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Listen before the graph runs, so an ability that ends inside its own activation still cleans up.
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	StopListeningForInterruption(ASC);
	if (ASC)
		for (const FGameplayTag& Tag : InterruptionTags)
			InterruptionTagDelegateHandles.Add(Tag, ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnInterruptionTagChanged));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPWGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopListeningForInterruption(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPWGameplayAbilityBase::StopListeningForInterruption(UAbilitySystemComponent* ASC)
{
	if (ASC)
		for (const TPair<FGameplayTag, FDelegateHandle>& Pair : InterruptionTagDelegateHandles)
			ASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved).Remove(Pair.Value);
	InterruptionTagDelegateHandles.Reset();
}

void UPWGameplayAbilityBase::OnInterruptionTagChanged(FGameplayTag GameplayTag, int NewCount)
{
	if (NewCount > 0 && IsActive() && CanBeCanceled())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicate*/true);
	}
}

UPWAbilitySystemComponent* UPWGameplayAbilityBase::GetPWAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UPWAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

AController* UPWGameplayAbilityBase::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

#if WITH_EDITOR

void UPWGameplayAbilityBase::SyncIdentityIntoAssetTags()
{
	if (AbilityTag.IsValid() && !GetAssetTags().HasTagExact(AbilityTag))
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		AbilityTags.AddTag(AbilityTag);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
	// The cooldown check blocks on the ability's cooldown tag, so older abilities that also listed it here drop it.
	const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
	if (CooldownTag.IsValid())
		ActivationBlockedTags.RemoveTag(CooldownTag);
}

FString UPWGameplayAbilityBase::GetIdentityTagName() const
{
	FString ClassName = GetClass()->GetName();
	ClassName.RemoveFromEnd(TEXT("_C"));
	if (ClassName.StartsWith(TEXT("BP_"))) { ClassName.RightChopInline(3); }
	if (ClassName.StartsWith(TEXT("GA_"))) { ClassName.RightChopInline(3); }

	return FString::Printf(TEXT("Ability.Skill.%s"), *ClassName);
}

void UPWGameplayAbilityBase::GenerateIdentityTag()
{
	const FGameplayTag IdentityTag = UGameplayTagsManager::Get().RequestGameplayTag(*GetIdentityTagName(), false);

	Modify();

	if (IdentityTag.IsValid())
	{
		AbilityTag = IdentityTag;
	}

	SyncIdentityIntoAssetTags();

	PostEditChange();
	MarkPackageDirty();
}

void UPWGameplayAbilityBase::PostLoad()
{
	Super::PostLoad();
	SyncIdentityIntoAssetTags();
}

void UPWGameplayAbilityBase::PostEditChangeProperty(FPropertyChangedEvent& E)
{
	Super::PostEditChangeProperty(E);
	SyncIdentityIntoAssetTags();
}

#endif
