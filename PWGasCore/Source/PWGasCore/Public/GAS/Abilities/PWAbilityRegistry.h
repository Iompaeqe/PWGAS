// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PWAbilityRegistry.generated.h"

class UPWAbilitySystemComponent;
class UPWAbilityDataAsset;
struct FPWAbilityInfo;

UCLASS()
class PWGASCORE_API UPWAbilityRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Every ability that can be granted by tag. The project's registry is set in Project Settings; a game mode can bring its own as a PW Ability Registry Provider.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UPWAbilityDataAsset>> Abilities;

	// This world's registry: its game mode's when it provides one, else the project's.
	UFUNCTION(BlueprintPure, Category = "Ability", meta=(WorldContext="WorldContextObject"))
	static UPWAbilityRegistry* Get(const UObject* WorldContextObject);

	// Looks an ability up in this world's registry.
	UFUNCTION(BlueprintPure, Category = "Ability", meta=(WorldContext="WorldContextObject"))
	static FPWAbilityInfo FindAbility(const UObject* WorldContextObject, FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable)
	FPWAbilityInfo GetAbilityByTag(const FGameplayTag& AbilityTag);

	// Loads every listed ability now, so the first lookup, such as UI at the start of a match, doesn't wait on loading.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void Preload();

#if WITH_EDITOR
	// Whether any registry asset lists the tag. Ability sets can be used in any world, so the editor checks them all.
	static bool IsInAnyRegistry(const FGameplayTag& AbilityTag);

	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override;
#endif

	//Example function to generate description text
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void MakeDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription);
	//Example function to generate description text for the next level
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void MakeNextDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription);

	
private:
	UPWAbilityDataAsset* FindData(const FGameplayTag& AbilityTag);

	// Tag to ability, built on the first lookup.
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UPWAbilityDataAsset>> Cache;
	bool bCached = false;

	//Example function to get description text
	virtual FText GetDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level);
	//Example function to get description text for the next level
	virtual FText GetNextDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level);
	//Example function to get description text for locked ability
	virtual FText GetLockedDescription(int32 Level);
	
	//Example function to format the values for the description texts
	virtual FText FormatTextValues(const FGameplayTag& AbilityTag, int32 Level, const FText& Description, const bool Next = false);
};
