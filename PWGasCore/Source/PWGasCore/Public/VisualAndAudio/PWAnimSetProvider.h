// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PWAnimSetProvider.generated.h"

UINTERFACE()
class UPWAnimSetProvider : public UInterface
{
	GENERATED_BODY()
};

class PWGASCORE_API IPWAnimSetProvider
{
	GENERATED_BODY()

public:
	
	/**
	 * Should return the animation for a character class
	 * @param AnimTag Animation GameplayTag that should be associated with a UAnimMontage
	 * @param OutAnimRate Animation play rate, intended for dynamic animation speed used with attributes such as "Cast Speed".
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Animation")
	UAnimMontage* GetMontageForTag(FGameplayTag AnimTag, float& OutAnimRate) const;
};
