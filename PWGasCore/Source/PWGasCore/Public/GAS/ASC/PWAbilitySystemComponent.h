// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "PWASC_AbilityLifecycle.h"
#include "PWASC_CooldownHandler.h"
#include "PWASC_DataManagement.h"
#include "PWASC_EffectLifecycle.h"
#include "PWASC_InputBinding.h"
#include "PWASC_VfxLifecycle.h"
#include "PWAbilitySystemComponent.generated.h"

struct FPWNiagaraEntry;
class UPWAbilitySet;
struct FPWASCVfxParams;
class UNiagaraComponent;
enum class EInputEventType : uint8;
DECLARE_MULTICAST_DELEGATE_TwoParams(FAnyGameplayTagChangedDelegate, FGameplayTag& /*Tag*/, int /*Count*/);

UCLASS(ClassGroup=(GASCore), meta=(BlueprintSpawnableComponent))
class PWGASCORE_API UPWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UPWAbilitySystemComponent();

	FAnyGameplayTagChangedDelegate AnyGameplayTagChanged; //server and client

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRep_ActivateAbilities() override;

	FPWASC_DataManagement& Data() const { return *DataManagement; }
	FPWASC_AbilityLifecycle& Abilities() const { return *AbilityLifecycle; }
	FPWASC_EffectLifecycle& Effects() const { return *EffectLifecycle; }
	FPWASC_InputBinding& Input() const { return *InputBinding; }
	FPWASC_CooldownHandler& Cooldown() const { return *CooldownHandler; }
	FPWASC_VfxLifecycle& Vfx() const { return *VfxLifecycle; }

	//Ability Helpers
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Add Abilities"))
	void BP_AddAbilities(const FGameplayTagContainer& AbilityTags);
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Add Ability Set"))
	void BP_AddAbilitySet(const UPWAbilitySet* AbilitySet);
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Add Ability"))
	void BP_AddAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag = FGameplayTag());
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Remove Ability"))
	void BP_RemoveAbility(const FGameplayTag& AbilityTag);
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Upgrade Ability"))
	void BP_UpgradeAbility(const FGameplayTag& AbilityTag);
	UFUNCTION(BlueprintCallable, Category="GASCore|Ability", meta=(DisplayName="Downgrade Ability"))
	void BP_DowngradeAbility(const FGameplayTag& AbilityTag);

	//Cooldown Helpers
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="GetCooldown Remaining For Tag"))
	bool BP_GetCooldownRemainingForTag(const FGameplayTag& CooldownTag, float& TimeRemaining, float& CooldownDuration) const;
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="GetCooldown Remaining For Ability Tag"))
	bool BP_GetCooldownRemainingForAbilityTag(const FGameplayTag& AbilityTag, float& TimeRemaining, float& TotalDuration) const;
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="Clear Cooldown For Tag"))
	int32 BP_ClearCooldownByTag(const FGameplayTag& CooldownTag);
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="Clea rCooldown For Ability Tag"))
	int32 BP_ClearCooldownForAbilityTag(const FGameplayTag& AbilityTag);
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="Adjust Cooldown For Tag"))
	bool BP_AdjustCooldownForTag(const FGameplayTag& CooldownTag, float SecondsToReduce);
	UFUNCTION(BlueprintCallable, Category="GASCore|Cooldown", meta=(DisplayName="Adjust Cooldown For Ability Tag"))
	bool BP_AdjustCooldownForAbilityTag(const FGameplayTag& AbilityTag, float SecondsToReduce);

	UFUNCTION(Server, Reliable)
	void ServerSendInputEvent(const FGameplayTag& InputTag, EInputEventType EventType);
	void DispatchInputEvent(const FGameplayTag& InputTag, EInputEventType EventType);
	UFUNCTION(BlueprintCallable, Category="GASCore|Input")
	void SendInputEvent(const FGameplayTag& InputTag, EInputEventType EventType);
	
	UFUNCTION(NetMulticast, Reliable, Category="GASCore|NiagaraVfx")
	void Multicast_PlayNiagara(const FPWNiagaraEntry& Entry);
	UFUNCTION(NetMulticast, Reliable, Category="GASCore|NiagaraVfx")
	void Multicast_StopNiagara(const TArray<int32>& Ids);
	
protected:
	virtual TUniquePtr<FPWASC_DataManagement> CreateDataManagement();
	virtual TUniquePtr<FPWASC_AbilityLifecycle> CreateAbilityLifecycle();
	virtual TUniquePtr<FPWASC_EffectLifecycle> CreateEffectLifecycle();
	virtual TUniquePtr<FPWASC_InputBinding> CreateInputBinding();
	virtual TUniquePtr<FPWASC_CooldownHandler> CreateCooldownHandler();
	virtual TUniquePtr<FPWASC_VfxLifecycle> CreateVfxLifecycle();

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	
private:
	TUniquePtr<FPWASC_DataManagement> DataManagement;
	TUniquePtr<FPWASC_AbilityLifecycle> AbilityLifecycle;
	TUniquePtr<FPWASC_EffectLifecycle> EffectLifecycle;
	TUniquePtr<FPWASC_InputBinding> InputBinding;
	TUniquePtr<FPWASC_CooldownHandler> CooldownHandler;
	TUniquePtr<FPWASC_VfxLifecycle> VfxLifecycle;

};
