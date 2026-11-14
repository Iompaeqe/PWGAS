// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PWMontageSet.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWMontageEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage", meta=(ClampMin="0.01"))
	float PlayRate = 1.f;
};

// The montages one kind of body plays for each animation tag. Bodies that share a skeleton can share a set.
UCLASS(BlueprintType)
class PWGASCORE_API UPWMontageSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montages", meta=(Categories="AnimMontage", ForceInlineRow))
	TMap<FGameplayTag, FPWMontageEntry> Montages;

	UFUNCTION(BlueprintPure, Category="Montages", meta=(GameplayTagFilter="AnimMontage"))
	UAnimMontage* FindMontage(FGameplayTag AnimTag, float& OutPlayRate) const;
};
