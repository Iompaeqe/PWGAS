// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/PWGameplayAbilityBase.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayCueManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "PWGASCoreLogChannels.h"
#include "Ability/PWSpellParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"
#include "VisualAndAudio/PWAnimSetProvider.h"



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
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AnimationTags.IsValidIndex(Index) || !AnimationTags[Index].IsValid())
		return;

	if (AvatarActor->GetClass()->ImplementsInterface(UPWAnimSetProvider::StaticClass()))
	{
		OutMontage = IPWAnimSetProvider::Execute_GetMontageForTag(AvatarActor, AnimationTags[Index], OutAnimRate);
		return;
	}

	UE_LOG(LogPWGASCoreAbility, Warning, TEXT("UPWGameplayAbilityBase::GetAnimMontageFromActor: AvatarActor: %s does not use the IPWAnimSetProvider, See UPWGameplayAbilityBase::GetAnimationMontageFromActor"), *AvatarActor->GetName())
}

UAnimInstance* UPWGameplayAbilityBase::GetAnimInstanceFromActor() const
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		UE_LOG(LogPWGASCoreAbility, Log, TEXT("UPWGameplayAbilityBase::GetAnimInstanceFromActor: Actor is not Character, overwrite this function "));
		return nullptr;
	}
	if (UAnimInstance* AI = Character->GetMesh()->GetAnimInstance())
	{
		return AI;
	}
	UE_LOG(LogPWGASCoreAbility, Log, TEXT("UPWGameplayAbilityBase::GetAnimInstanceFromActor: Could not get UAnimInstance from Character"));
	return nullptr;
}

void UPWGameplayAbilityBase::ExecuteCueAtSocket(const FGameplayTag CueTag, const FName SocketName, const FVector Offset, const FRotator RotationOffset)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Avatar || !ASC) return;

	const USkeletalMeshComponent* Mesh = GetOwningComponentFromActorInfo();
	const bool bHasSocket = Mesh && Mesh->DoesSocketExist(SocketName);
	const FVector Origin = bHasSocket ? Mesh->GetSocketLocation(SocketName) : Avatar->GetActorLocation();

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

	// A tag that would cancel the ability also stops it from starting.
	if (ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(CancellationTags))
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

FGameplayTagContainer UPWGameplayAbilityBase::GetCooldownGameplayTags() const
{
	FGameplayTagContainer MutableTags = FGameplayTagContainer();
	MutableTags.Reset();

	if (const FGameplayTagContainer* Parent = Super::GetCooldownTags())
		MutableTags.AppendTags(*Parent);

	if (AbilityTag.IsValid())
	{
		const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
		if (CooldownTag.IsValid())
			MutableTags.AddTag(CooldownTag);
	}

	return MutableTags;
}

void UPWGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	float Cd = Cooldown.GetValueAtLevel(GetAbilityLevel());
	if (const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect())
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTag.GetSingleTagContainer());
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(PWTags::Ability::SetByCaller::Cooldown, Cd);
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
	StopListeningForCancellation(ASC);
	if (ASC)
		for (const FGameplayTag& Tag : CancellationTags)
			CancellationTagDelegateHandles.Add(Tag, ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnCancellationTagChanged));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPWGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopListeningForCancellation(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPWGameplayAbilityBase::StopListeningForCancellation(UAbilitySystemComponent* ASC)
{
	if (ASC)
		for (const TPair<FGameplayTag, FDelegateHandle>& Pair : CancellationTagDelegateHandles)
			ASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved).Remove(Pair.Value);
	CancellationTagDelegateHandles.Reset();
}

void UPWGameplayAbilityBase::OnCancellationTagChanged(FGameplayTag GameplayTag, int NewCount)
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
	const FName CooldownFName(*(AbilityTag.ToString() + TEXT(".Cooldown")));
	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(CooldownFName, false);
	if (CooldownTag.IsValid() && !ActivationBlockedTags.HasTagExact(CooldownTag))
	{
		ActivationBlockedTags.AddTag(CooldownTag);
	}
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
	const FString IdentityStr = GetIdentityTagName();
	const FString CooldownStr = IdentityStr + TEXT(".Cooldown");

	const FGameplayTag IdentityTag = UGameplayTagsManager::Get().RequestGameplayTag(*IdentityStr, false);
	const FGameplayTag CooldownTag = UGameplayTagsManager::Get().RequestGameplayTag(*CooldownStr, false);

	Modify();

	if (IdentityTag.IsValid())
	{
		AbilityTag = IdentityTag;
	}
	if (CooldownTag.IsValid() && !ActivationBlockedTags.HasTagExact(CooldownTag))
	{
		ActivationBlockedTags.AddTag(CooldownTag);
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
