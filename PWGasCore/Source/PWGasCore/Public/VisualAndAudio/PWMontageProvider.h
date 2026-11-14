// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PWMontageProvider.generated.h"

class UAnimMontage;

UINTERFACE(BlueprintType, Blueprintable)
class UPWMontageProvider : public UInterface
{
	GENERATED_BODY()
};

// Picks the montage an actor plays for an animation tag, so abilities only name animations and never care what casts them.
class PWGASCORE_API IPWMontageProvider
{
	GENERATED_BODY()

public:
	// The actor itself when it implements this, otherwise its first component that does, such as a PW Montage Set.
	static const UObject* Find(const AActor* Actor);

	/**
	 * @param AnimTag Animation tag an ability asks for.
	 * @param OutAnimRate Play rate, for things like a cast speed attribute.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Animation")
	UAnimMontage* GetMontageForTag(FGameplayTag AnimTag, float& OutAnimRate) const;
};
