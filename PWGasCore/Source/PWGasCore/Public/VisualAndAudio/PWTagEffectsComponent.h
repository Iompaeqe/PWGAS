// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "PWTagEffectsComponent.generated.h"

class UAbilitySystemComponent;
class UMaterialInterface;
class UMeshComponent;
class UNiagaraComponent;
class UPWTagEffectSet;
struct FPWTagEffectNiagara;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPWTagEffectEvent, FGameplayTag, Tag);

// Plays the effect set's Niagara and overlay material on this actor while its ability system has the matching tag. Cosmetic only, so it does nothing on a dedicated server.
UCLASS(ClassGroup=(GASCore), Blueprintable, meta=(BlueprintSpawnableComponent, DisplayName="PW Tag Effects"))
class PWGASCORE_API UPWTagEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWTagEffectsComponent();

	// Listens to this ability system from now on. Done at begin play when the actor already has one; an actor whose ability system arrives later, such as a character using its player state's, calls it then.
	UFUNCTION(BlueprintCallable, Category="Tag Effects")
	void SetAbilitySystem(UAbilitySystemComponent* AbilitySystem);

	// Swaps the set; what is on now restarts with the new set's effects.
	UFUNCTION(BlueprintCallable, Category="Tag Effects")
	void SetEffectSet(UPWTagEffectSet* NewSet);

	UFUNCTION(BlueprintPure, Category="Tag Effects")
	UPWTagEffectSet* GetEffectSet() const { return EffectSet; }

	// A tag listed in the effect set turned on. For small reactions of your own, such as shaking a totem while it casts; an entry with no Niagara and no material only fires these.
	UPROPERTY(BlueprintAssignable, Category="Tag Effects")
	FPWTagEffectEvent OnTagAdded;

	UPROPERTY(BlueprintAssignable, Category="Tag Effects")
	FPWTagEffectEvent OnTagRemoved;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tag Effects")
	TObjectPtr<UPWTagEffectSet> EffectSet;

private:
	void OnTagChanged(FGameplayTag Tag, int32 Count);
	void StartEffect(const FGameplayTag& Tag);
	void SpawnNiagara(const FPWTagEffectNiagara& Entry, FName Socket, TArray<TWeakObjectPtr<UNiagaraComponent>>& Spawned) const;
	void StopEffect(const FGameplayTag& Tag);
	void StartPresentEffects();
	void StopAllEffects();
	void ReleaseAbilitySystem();
	void RefreshOverlay();
	bool IsCosmetic() const;

	TWeakObjectPtr<UAbilitySystemComponent> BoundAbilitySystem;
	FDelegateHandle TagChangedHandle;

	TMap<FGameplayTag, TArray<TWeakObjectPtr<UNiagaraComponent>>> ActiveNiagara;

	// Tags with an overlay material that are on, latest last.
	TArray<FGameplayTag> OverlayOrder;
	TMap<TWeakObjectPtr<UMeshComponent>, TWeakObjectPtr<UMaterialInterface>> OriginalOverlays;
};
