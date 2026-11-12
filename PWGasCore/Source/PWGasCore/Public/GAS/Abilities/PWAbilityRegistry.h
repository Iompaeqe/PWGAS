// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PWAbilityRegistry.generated.h"

class UPWAbilitySystemComponent;
class UAbilityDataAsset;
struct FPWAbilityInfo;

UCLASS()
class PWGASCORE_API UPWAbilityRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAbilityDataAsset>> Abilities;
	UFUNCTION(BlueprintCallable)
	FPWAbilityInfo GetAbilityByTag(const FGameplayTag& AbilityTag);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	//Example function to generate description text
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void MakeDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription);
	//Example function to generate description text for the next level
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void MakeNextDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription);

	
private:
	//Example function to get description text
	virtual FText GetDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level);
	//Example function to get description text for the next level
	virtual FText GetNextDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level);
	//Example function to get description text for locked ability
	virtual FText GetLockedDescription(int32 Level);
	
	//Example function to format the values for the description texts
	virtual FText FormatTextValues(const FGameplayTag& AbilityTag, int32 Level, const FText& Description, const bool Next = false);
};
