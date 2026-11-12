// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PWGameplayAbilityBase.h"
#include "Modules/PWAbilityModule.h"
#include "Modules/DataModules/PWDataModule.h"
#include "Modules/ControlModules/PWControlModule.h"
#include "Modules/ActionModules/PWAbilityModule_Aura.h"
#include "Modules/ActionModules/PWAbilityModule_InstantEffect.h"
#include "Modules/ActionModules/PWAbilityModule_MultiActor.h"
#include "Modules/ActionModules/PWAbilityModule_MultiProjectile.h"
#include "Modules/ControlModules/PWAbilityModule_Niagara.h"
#include "Modules/ControlModules/PWAbilityModule_Precast.h"
#include "Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "Modules/DataModules/PWAbilityModule_Effect.h"
#include "Modules/DataModules/PWAbilityModule_Radius.h"
#include "Modules/DataModules/PWAbilityModule_Range.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWModularGameplayAbility.generated.h"

class UPWActionModule;

UENUM(BlueprintType)
enum class EPWEndReason : uint8
{
	Completed,
	Cancelled,
	Interrupted
};

// When an ability pays its cost and starts its cooldown.
UENUM(BlueprintType)
enum class EPWCommitPolicy : uint8
{
	Manual,
	OnStart,
	OnEnd
};

UCLASS(Blueprintable)
class PWGASCORE_API UPWModularGameplayAbility : public UPWGameplayAbilityBase
{
	GENERATED_BODY()

public:
	// == Modules  == //
	// ============== //

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Ability|Modules")
	TObjectPtr<UPWActionModule> ActionModule;
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Ability|Modules")
	TArray<TObjectPtr<UPWDataModule>> DataModules;
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Ability|Modules")
	TArray<TObjectPtr<UPWControlModule>> ControlModules;

	UFUNCTION(BlueprintCallable, BlueprintPure,Category="Ability|Modules",meta=(DeterminesOutputType="ModuleClass"))
	UPWAbilityModule* GetModule(TSubclassOf<UPWAbilityModule> ModuleClass) const;
	
	template <typename T>
	T* GetActionModule() const
	{
		return Cast<T>(ActionModule);
	}

	template <typename T>
	T* GetDataModule() const
	{
		for (UPWDataModule* Mod : DataModules)
			if (Mod && Mod->IsA(T::StaticClass()))
				return Cast<T>(Mod);

		return nullptr;
	}

	template <typename T>
	T* GetControlModule() const
	{
		for (UPWControlModule* Mod : ControlModules)
			if (Mod && Mod->IsA(T::StaticClass()))
				return Cast<T>(Mod);

		return nullptr;
	}

	void ForEachModule(TFunctionRef<void(UPWAbilityModule*)> Callback);

	virtual bool DefersAction() const { return false; }

	//Action Modules
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Actor* GetActorModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Aura* GetAuraModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_InstantEffect* GetInstantEffectModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_MultiActor* GetMultiActorModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_MultiProjectile* GetMultiProjectileModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Projectile* GetProjectileModule() const;
	//Control Modules
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Niagara* GetNiagaraModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Precast* GetPrecastModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Targeting* GetTargetingModule() const;
	//Data Modules
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Effect* GetEffectModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Radius* GetRadiusModule() const;
	UFUNCTION(BlueprintPure, Category="Modules")
	UPWAbilityModule_Range* GetRangeModule() const;

	// == Gameplay Targeting  == //
	// ========================= //

	UFUNCTION(BlueprintCallable, Category="Targeting")
	FPWTargetingResult ComputeTargetOnce() const;

	// Build TargetData from targeting result
	UFUNCTION(BlueprintCallable, Category="Targeting")
	FGameplayAbilityTargetDataHandle MakeTargetDataFromResult(const FPWTargetingResult& Result) const;

	// Lightweight extractor for a single TargetData entry
	FVector ExtractLocation(const FGameplayAbilityTargetData* TD) const;

protected:
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// == Modules  == //
	// ============== //

	void EnsureRequiredDataModules();

private:
	static TMap<UClass*, TArray<UPWAbilityModule*>> CachedByType;

	
#if WITH_EDITOR
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override;
#endif
};
