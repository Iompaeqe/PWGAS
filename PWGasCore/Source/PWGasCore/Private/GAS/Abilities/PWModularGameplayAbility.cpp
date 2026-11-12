// copyright mkurtt96


#include "GAS/Abilities/PWModularGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_InstantEffect.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiActor.h"
#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "PWGasCore/Public/GAS/Abilities/Modules/PWAbilityModule.h"
#include "Targeting/Data/PWTargetingData.h"


void UPWModularGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ForEachModule([this](UPWAbilityModule* Mod)
	{
		Mod->Initialize(this);
	});
	ForEachModule([](UPWAbilityModule* Mod)
	{
		Mod->OnAbilityActivated();
	});
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UPWAbilityModule* UPWModularGameplayAbility::GetModule(const TSubclassOf<UPWAbilityModule> ModuleClass) const
{
	if (!*ModuleClass)
		return nullptr;

	// Action
	if (ActionModule && ActionModule->IsA(ModuleClass))
		return ActionModule;

	// Data
	for (UPWDataModule* Mod : DataModules)
		if (Mod && Mod->IsA(ModuleClass))
			return Mod;

	// Control
	for (UPWControlModule* Mod : ControlModules)
		if (Mod && Mod->IsA(ModuleClass))
			return Mod;

	return nullptr;
}

FPWTargetingResult UPWModularGameplayAbility::ComputeTargetOnce() const
{
	if (UPWAbilityModule_Targeting* TMod = GetControlModule<UPWAbilityModule_Targeting>())
	{
		FPWTargetingResult R;
		TMod->ComputeTarget(R);
		return R;
	}
	return FPWTargetingResult();
}

FGameplayAbilityTargetDataHandle UPWModularGameplayAbility::MakeTargetDataFromResult(const FPWTargetingResult& Result) const
{
	if (Result.bHasHit && Result.Hit.bBlockingHit)
	{
		FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit(Result.Hit);
		return FGameplayAbilityTargetDataHandle(HitData);
	}

	FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
	LocData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocData->TargetLocation.LiteralTransform = FTransform(Result.Rotation, Result.Location);
	return FGameplayAbilityTargetDataHandle(LocData);
}

FVector UPWModularGameplayAbility::ExtractLocation(const FGameplayAbilityTargetData* TD) const
{
	if (!TD) return FVector::ZeroVector;

	if (TD->HasHitResult() && TD->GetHitResult()->IsValidBlockingHit())
		return TD->GetHitResult()->ImpactPoint;
	if (TD->HasEndPoint())
		return TD->GetEndPoint();
	return FVector::ZeroVector;
}

void UPWModularGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UPWModularGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	ForEachModule([this](UPWAbilityModule* Mod)
	{
		Mod->OnAbilityEnded();
	});
}

void UPWModularGameplayAbility::EnsureRequiredDataModules()
{
	TArray<TSubclassOf<UPWDataModule>> Required;
	ForEachModule([&Required](UPWAbilityModule* Module)
	{
		Module->GetRequiredDataModules(Required);
	});

	for (TSubclassOf<UPWDataModule> DataClass : Required)
	{
		if (!DataClass)
			continue;

		bool bExists = false;
		for (UPWDataModule* M : DataModules)
			if (M && M->IsA(DataClass))
			{
				bExists = true;
				break;
			}

		if (!bExists)
		{
			UPWDataModule* NewData = NewObject<UPWDataModule>(this, DataClass, NAME_None, RF_Transactional);
			DataModules.Add(NewData);
			//NewData->Initialize(this);
			UE_LOG(LogTemp, Log, TEXT("[PWGASCore] Auto-added required DataModule: %s"), *DataClass->GetName());
		}
	}
}

void UPWModularGameplayAbility::ForEachModule(TFunctionRef<void(UPWAbilityModule*)> Callback)
{
	if (ActionModule)
		Callback(ActionModule);

	for (UPWDataModule* M : DataModules)
		if (M)
			Callback(M);

	for (UPWControlModule* M : ControlModules)
		if (M)
			Callback(M);
}

UPWAbilityModule_Actor* UPWModularGameplayAbility::GetActorModule() const
{
	return GetActionModule<UPWAbilityModule_Actor>();
}

UPWAbilityModule_Aura* UPWModularGameplayAbility::GetAuraModule() const
{
	return GetActionModule<UPWAbilityModule_Aura>();
}

UPWAbilityModule_InstantEffect* UPWModularGameplayAbility::GetInstantEffectModule() const
{
	return GetActionModule<UPWAbilityModule_InstantEffect>();
}

UPWAbilityModule_MultiActor* UPWModularGameplayAbility::GetMultiActorModule() const
{
	return GetActionModule<UPWAbilityModule_MultiActor>();
}

UPWAbilityModule_MultiProjectile* UPWModularGameplayAbility::GetMultiProjectileModule() const
{
	return GetActionModule<UPWAbilityModule_MultiProjectile>();
}

UPWAbilityModule_Projectile* UPWModularGameplayAbility::GetProjectileModule() const
{
	return GetActionModule<UPWAbilityModule_Projectile>();
}

UPWAbilityModule_Niagara* UPWModularGameplayAbility::GetNiagaraModule() const
{
	return GetControlModule<UPWAbilityModule_Niagara>();
}

UPWAbilityModule_Precast* UPWModularGameplayAbility::GetPrecastModule() const
{
	return GetControlModule<UPWAbilityModule_Precast>();
}

UPWAbilityModule_Targeting* UPWModularGameplayAbility::GetTargetingModule() const
{
	return GetControlModule<UPWAbilityModule_Targeting>();
}

UPWAbilityModule_Effect* UPWModularGameplayAbility::GetEffectModule() const
{
	return GetDataModule<UPWAbilityModule_Effect>();
}

UPWAbilityModule_Radius* UPWModularGameplayAbility::GetRadiusModule() const
{
	return GetDataModule<UPWAbilityModule_Radius>();
}

UPWAbilityModule_Range* UPWModularGameplayAbility::GetRangeModule() const
{
	return GetDataModule<UPWAbilityModule_Range>();
}


#if WITH_EDITOR
void UPWModularGameplayAbility::PostEditChangeProperty(FPropertyChangedEvent& E)
{
	Super::PostEditChangeProperty(E);
	EnsureRequiredDataModules();
}
#endif