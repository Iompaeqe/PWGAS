// copyright mkurtt96


#include "GAS/Abilities/PWGameplayAbilityBase.h"

#include "AbilitySystemLog.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "PWGASCoreLogChannels.h"
#include "Ability/SpellParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"
#include "VisualAndAudio/PWAnimSetProvider.h"


UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_Message_Cooldown, "Ability.Message.Cooldown");

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
}

float UPWGameplayAbilityBase::GetCooldown(int32 Level) const
{
	return Cooldown.GetValueAtLevel(Level);
}

void UPWGameplayAbilityBase::GetCost(int32 Level, TArray<FCostData>& CostData) const
{
	UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (!CostGE) return;

	for (FGameplayModifierInfo Modifier : CostGE->Modifiers)
	{
		float Value = 0.f;
		Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(Level, Value);
		FCostData Data = FCostData(Modifier.Attribute.AttributeName, Modifier.ModifierOp, Value);
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

FGameplayTag UPWGameplayAbilityBase::GetActivationInputTag() const
{
	for (auto Tag : GetCurrentAbilitySpec()->GetDynamicSpecSourceTags())
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

USpellParams* UPWGameplayAbilityBase::MakeSpellParams()
{
	USpellParams* Params = NewObject<USpellParams>(this);
	Params->SourceAvatar = GetAvatarActorFromActorInfo();
	Params->AbilityLevel = GetAbilityLevel();

	if (const APlayerState* PS = GetPawn() ? GetPawn()->GetPlayerState() : nullptr)
		Params->SourcePlayerUniqueId = PS->GetUniqueId();

	if (!Params->More)
		Params->More = NewObject<UMultiDataArray>(Params);

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

	bool bIsNetAuthority = ActorInfo->IsNetAuthority();
	UE_LOG(LogGameplayTags, Log, TEXT("bIsNetAuthority %hs"), bIsNetAuthority? "true":"false");

	if (const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo())
	{
		if (Info->IsLocallyControlled())
		{
			FCooldownMessage Message = FCooldownMessage();
			Message.ASC = Cast<UPWAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
			Message.AbilityTag = AbilityTag;
			Message.Duration = Cd;
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Tag_Ability_Message_Cooldown, Message);
		}
	}
}

void UPWGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		for (const FGameplayTag& Tag : CancellationTags)
		{
			auto& Event = ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved);
			FDelegateHandle DelegateHandle = Event.AddUObject(this, &ThisClass::OnCancellationTagChanged);
			CancellationTagDelegateHandles.Add(DelegateHandle);
		}
	}
}

void UPWGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		for (const FGameplayTag& Tag : CancellationTags)
		{
			auto& Event = ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved);
			for (FDelegateHandle& DelegateHandle : CancellationTagDelegateHandles)
			{
				Event.Remove(DelegateHandle);
			}
		}
	}


	CancellationTagDelegateHandles.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPWGameplayAbilityBase::OnCancellationTagChanged(FGameplayTag GameplayTag, int NewCount)
{
	if (NewCount > 0 && CanBeCanceled())
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

void UPWGameplayAbilityBase::AddTagIfMissing(UGameplayTagsSettings* Settings, const FName& TagFName, const FString& Comment)
{
	if (!UGameplayTagsManager::Get().RequestGameplayTag(TagFName, /*ErrorIfNotFound*/false).IsValid())
	{
		FGameplayTagTableRow Row;
		Row.Tag = TagFName;
		Row.DevComment = Comment;
		Settings->GameplayTagList.Add(Row);
	}
}

bool UPWGameplayAbilityBase::EnsureProjectTagExists(const FString& Tag, const FString& Comment)
{
	if (UGameplayTagsSettings* Settings = GetMutableDefault<UGameplayTagsSettings>())
	{
		const FName IdentityFName(*Tag);
		const FName CooldownFName(*(Tag + TEXT(".Cooldown")));

		AddTagIfMissing(Settings, IdentityFName, Comment);
		AddTagIfMissing(Settings, CooldownFName, Comment);
		AddTagIfMissing(Settings, CooldownFName, Comment);

		Settings->SaveConfig();
		Settings->TryUpdateDefaultConfigFile();
		UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
		return true;
	}
	return false;
}

void UPWGameplayAbilityBase::GenerateIdentityTag()
{
	FString ClassName = GetClass()->GetName();
	ClassName.RemoveFromEnd(TEXT("_C"));
	if (ClassName.StartsWith(TEXT("BP_"))) { ClassName.RightChopInline(3); }
	if (ClassName.StartsWith(TEXT("GA_"))) { ClassName.RightChopInline(3); }

	const FString IdentityStr = FString::Printf(TEXT("Ability.Skill.%s"), *ClassName);
	const FString CooldownStr = IdentityStr + TEXT(".Cooldown");

	EnsureProjectTagExists(IdentityStr, TEXT("Auto-generated by GASCore"));

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
