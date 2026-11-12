// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "PWGameplayAbilityBase.generated.h"

class USpellParams;
class UPWAbilitySystemComponent;
class UGameplayTagsSettings;


UENUM(BlueprintType)
enum class EPWActivationGroup : uint8
{
	Independent = 0,
	Exclusive_Replaceable = 1,
	Exclusive_Blocking = 2,
	MAX = 3	UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct PWGASCORE_API FCostData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString AttributeName;
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EGameplayModOp::Type> Modifier = EGameplayModOp::AddBase;
	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;

	FCostData() = default;
	FCostData(const FString& AttributeName, const EGameplayModOp::Type Modifier, const float Value)
	{
		this->AttributeName = AttributeName;
		this->Modifier = Modifier;
		this->Value = Value;
	}
};

UCLASS()
class PWGASCORE_API UPWGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPWGameplayAbilityBase();
	
	// The ability's identity tag; its cooldown tag is this tag plus .Cooldown.
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta=(GameplayTagFilter = "Ability.Skill"))
	FGameplayTag AbilityTag;
	// Montage tags the avatar resolves through its anim set provider; Get Anim Montage From Actor picks one by index.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation", meta=(GameplayTagFilter = "AnimMontage"))
	TArray<FGameplayTag> AnimationTags;
	
	// Returns true if the requested activation group is a valid transition.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(EPWActivationGroup NewGroup) const;
	// Tries to change the activation group.  Returns true if it successfully changed.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(EPWActivationGroup NewGroup);

	// == Ability / Config == //
	// ====================== //

	USpellParams* MakeSpellParams();
	
	// == Utils  == //
	// ============ //

	APlayerController* GetPC() const;
	APawn* GetPawn() const;
	
protected:
	// Independent abilities run alongside anything, Replaceable ones are cancelled by the next exclusive ability, Blocking ones stop other exclusive abilities from starting.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	EPWActivationGroup ActivationGroup;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooldowns", meta=(ToolTip="Used with GE_Cooldown that handles cooldown tag and duration dynamically."))
	FScalableFloat Cooldown;
	// The ability is cancelled as soon as its owner gains any of these tags, and cannot start while one is held; for example a stun.
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer CancellationTags;
	
	//~UGameplayAbility interface
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface
	
	TMap<FGameplayTag, FDelegateHandle> CancellationTagDelegateHandles;
	void StopListeningForCancellation(UAbilitySystemComponent* ASC);
	void OnCancellationTagChanged(FGameplayTag GameplayTag, int NewCount);
	
private:

public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UPWAbilitySystemComponent* GetPWAbilitySystemComponentFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Ability")
	AController* GetControllerFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	virtual float GetCooldown(int32 Level) const;
	UFUNCTION(BlueprintCallable, Category = "Cost")
	virtual void GetCost(int32 Level, TArray<FCostData>& CostData) const;
	
	UFUNCTION(BlueprintPure, Category="Animation")
	void GetAnimMontageFromActor(UAnimMontage*& OutMontage, float& OutAnimRate, int Index = 0) const;
	UFUNCTION(BlueprintPure, Category="Animation")
	UAnimInstance* GetAnimInstanceFromActor() const;

	UFUNCTION(BlueprintCallable, Category="GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	void ExecuteCueAtSocket(FGameplayTag CueTag, FName SocketName, FVector Offset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator);
	UFUNCTION(BlueprintPure, Category = "Ability")
	FGameplayTag GetActivationInputTag() const;

	EPWActivationGroup GetActivationGroup() const { return ActivationGroup; }
	virtual FGameplayTagContainer GetCooldownGameplayTags() const;

	
#if WITH_EDITOR
	// == Editor Helpers  == //
	// ===================== //
	void SyncIdentityIntoAssetTags();

public:
	FString GetIdentityTagName() const;
	void GenerateIdentityTag();

protected:
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override;
#endif
	
};
