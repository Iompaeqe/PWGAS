// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/ASC/PWAbilitySystemComponent.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/ASC/PWASC_AbilityLifecycle.h"
#include "GAS/ASC/PWASC_DataManagement.h"
#include "GAS/ASC/PWASC_EffectLifecycle.h"
#include "GAS/ASC/PWASC_InputBinding.h"
#include "GAS/ASC/PWASC_VfxLifecycle.h"
#include "GAS/Data/GASCoreEnums.h"
#include "GAS/Data/PWNiagaraEntry.h"
#include "GAS/Tags/GASCoreTags.h"

UPWAbilitySystemComponent::UPWAbilitySystemComponent()
{
	DataManagement = nullptr;
	AbilityLifecycle = nullptr;
	EffectLifecycle = nullptr;
	InputBinding = nullptr;
	CooldownHandler = nullptr;
	VfxLifecycle = nullptr;
}

void UPWAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();

	if (!DataManagement) DataManagement = CreateDataManagement();
	if (!AbilityLifecycle) AbilityLifecycle = CreateAbilityLifecycle();
	if (!EffectLifecycle) EffectLifecycle = CreateEffectLifecycle();
	if (!InputBinding) InputBinding = CreateInputBinding();
	if (!CooldownHandler) CooldownHandler = CreateCooldownHandler();
	if (!VfxLifecycle) VfxLifecycle = CreateVfxLifecycle();

	GameplayTagCountContainer.RegisterGenericGameplayEvent().AddLambda([this](FGameplayTag Tag, int32 Count)
	{
		AnyGameplayTagChanged.Broadcast(Tag, Count);
	});
}

void UPWAbilitySystemComponent::OnUnregister()
{
	Super::OnUnregister();
	InputBinding.Reset();
	EffectLifecycle.Reset();
	AbilityLifecycle.Reset();
	DataManagement.Reset();
	CooldownHandler.Reset();
	VfxLifecycle.Reset();
}

void UPWAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPWAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (InAvatarActor && AbilityLifecycle && IsOwnerActorAuthoritative())
		AbilityLifecycle->ApplyActivationPolicies(PWTags::Ability::Activation::OnGranted);
}

void UPWAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
}

void UPWAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	AbilityLifecycle->AbilitiesUpdated.Broadcast();
}

void UPWAbilitySystemComponent::BP_AddAbilities(const FGameplayTagContainer& AbilityTags)
{
	AbilityLifecycle->AddAbilities(AbilityTags);
}

void UPWAbilitySystemComponent::BP_AddAbilitySet(const UPWAbilitySet* AbilitySet)
{
	AbilityLifecycle->AddAbilitySet(AbilitySet);
}

void UPWAbilitySystemComponent::BP_AddAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag)
{
	AbilityLifecycle->AddAbility(AbilityTag, InputTag);
}

void UPWAbilitySystemComponent::BP_RemoveAbility(const FGameplayTag& AbilityTag)
{
	AbilityLifecycle->RemoveAbility(AbilityTag);
}

void UPWAbilitySystemComponent::BP_UpgradeAbility(const FGameplayTag& AbilityTag)
{
	AbilityLifecycle->UpgradeAbility(AbilityTag);
}

void UPWAbilitySystemComponent::BP_DowngradeAbility(const FGameplayTag& AbilityTag)
{
	AbilityLifecycle->DowngradeAbility(AbilityTag);
}

bool UPWAbilitySystemComponent::BP_GetCooldownRemainingForTag(const FGameplayTag& CooldownTag, float& TimeRemaining, float& CooldownDuration) const
{
	return CooldownHandler->GetCooldownRemainingForTag(CooldownTag, TimeRemaining, CooldownDuration);
}

bool UPWAbilitySystemComponent::BP_GetCooldownRemainingForAbilityTag(const FGameplayTag& AbilityTag, float& TimeRemaining, float& TotalDuration) const
{
	return CooldownHandler->GetCooldownRemainingForAbilityTag(AbilityTag, TimeRemaining, TotalDuration);
}

int32 UPWAbilitySystemComponent::BP_ClearCooldownByTag(const FGameplayTag& CooldownTag)
{
	return CooldownHandler->ClearCooldownByTag(CooldownTag);
}

int32 UPWAbilitySystemComponent::BP_ClearCooldownForAbilityTag(const FGameplayTag& AbilityTag)
{
	return CooldownHandler->ClearCooldownForAbilityTag(AbilityTag);
}

bool UPWAbilitySystemComponent::BP_AdjustCooldownForTag(const FGameplayTag& CooldownTag, float SecondsToReduce)
{
	return CooldownHandler->AdjustCooldownForTag(CooldownTag, SecondsToReduce);
}

bool UPWAbilitySystemComponent::BP_AdjustCooldownForAbilityTag(const FGameplayTag& AbilityTag, float SecondsToReduce)
{
	return CooldownHandler->AdjustCooldownForAbilityTag(AbilityTag, SecondsToReduce);
}

void UPWAbilitySystemComponent::ServerSendInputEvent_Implementation(const FGameplayTag& InputTag, const EPWInputEventType EventType)
{
	if (InputTag.MatchesTag(PWTags::Input::Root))
		DispatchInputEvent(InputTag, EventType);
}

void UPWAbilitySystemComponent::DispatchInputEvent(const FGameplayTag& InputTag, const EPWInputEventType EventType)
{
	FGameplayEventData Payload;
	Payload.EventTag = PWTags::Event::Input;
	Payload.EventMagnitude = (EventType == EPWInputEventType::Pressed) ? 1.f : 0.f;
	Payload.InstigatorTags.AddTag(InputTag);
	HandleGameplayEvent(PWTags::Event::Input, &Payload);
}

void UPWAbilitySystemComponent::SendInputEvent(const FGameplayTag& InputTag, EPWInputEventType EventType)
{
	if (!InputTag.IsValid()) return;

	DispatchInputEvent(InputTag, EventType);

	if (!IsOwnerActorAuthoritative())
		ServerSendInputEvent(InputTag, EventType);
}

void UPWAbilitySystemComponent::Multicast_PlayNiagara_Implementation(const FPWNiagaraEntry& Entry)
{
	//UE_LOG(LogTemp, Log, TEXT("UPWAbilitySystemComponent::Multicast_PlayNiagara_Implementation, Auth: %hhd, UniqueId: %d"), GetOwner()->HasAuthority(), Entry.UniqueId);
	Vfx().PlayNiagara_Local(Entry);
}

void UPWAbilitySystemComponent::Multicast_StopNiagara_Implementation(const TArray<int32>& Ids)
{
	//for (int32 Id : Ids)
		//UE_LOG(LogTemp, Log, TEXT("UPWAbilitySystemComponent::Multicast_StopNiagara_Implementation, Auth: %hhd, UniqueId: %d"), GetOwner()->HasAuthority(), Id);
	Vfx().StopNiagaraList_Local(Ids);
}

TUniquePtr<FPWASC_DataManagement> UPWAbilitySystemComponent::CreateDataManagement()
{
	return MakeUnique<FPWASC_DataManagement>(*this);
}

TUniquePtr<FPWASC_AbilityLifecycle> UPWAbilitySystemComponent::CreateAbilityLifecycle()
{
	return MakeUnique<FPWASC_AbilityLifecycle>(*this);
}

TUniquePtr<FPWASC_EffectLifecycle> UPWAbilitySystemComponent::CreateEffectLifecycle()
{
	return MakeUnique<FPWASC_EffectLifecycle>(*this);
}

TUniquePtr<FPWASC_InputBinding> UPWAbilitySystemComponent::CreateInputBinding()
{
	return MakeUnique<FPWASC_InputBinding>(*this);
}

TUniquePtr<FPWASC_CooldownHandler> UPWAbilitySystemComponent::CreateCooldownHandler()
{
	return MakeUnique<FPWASC_CooldownHandler>(*this);
}

TUniquePtr<FPWASC_VfxLifecycle> UPWAbilitySystemComponent::CreateVfxLifecycle()
{
	return MakeUnique<FPWASC_VfxLifecycle>(*this);
}

void UPWAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (const UPWGameplayAbilityBase* PWAbility = Cast<UPWGameplayAbilityBase>(Ability))
		Abilities().AddAbilityToActivationGroup(PWAbility->GetActivationGroup(), PWAbility);
}

void UPWAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (const UPWGameplayAbilityBase* PWAbility = Cast<UPWGameplayAbilityBase>(Ability))
		Abilities().RemoveAbilityFromActivationGroup(PWAbility->GetActivationGroup(), PWAbility);
}


