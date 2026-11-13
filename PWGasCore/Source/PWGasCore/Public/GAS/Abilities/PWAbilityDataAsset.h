// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityInfo.h"
#include "Engine/DataAsset.h"
#include "PWAbilityDataAsset.generated.h"

UCLASS(BlueprintType)
class PWGASCORE_API UPWAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Everything the registry and UI need to know about one ability.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FPWAbilityInfo Ability;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
