// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWAttributeFunctions.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class PWATTRIBUTES_API UPWAttributeFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool ResolveAttributeOnASC(const UAbilitySystemComponent* ASC, const FGameplayTag& Tag, FGameplayAttribute& Out);
	
	UFUNCTION(BlueprintCallable, Category="PW|Attributes|Tags")
	static FGameplayAttribute GetAttribute(const UAbilitySystemComponent* ASC, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="PW|Attributes|Tags")
	static float GetAttributeValue(const UAbilitySystemComponent* ASC, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="PW|Attributes|Tags")
	static void SetAttributeValue(UAbilitySystemComponent* ASC, FGameplayTag Tag, float NewBase);
};
