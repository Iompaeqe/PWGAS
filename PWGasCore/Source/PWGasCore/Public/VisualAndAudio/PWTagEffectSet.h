// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PWTagEffectSet.generated.h"

class UMaterialInterface;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWTagEffectNiagara
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	TObjectPtr<UNiagaraSystem> System;

	// Sockets on any of the actor's meshes, skeletal or static; the system plays once at each. An empty list, None, or a socket the actor lacks uses the actor's root.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	TArray<FName> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	FVector Offset = FVector::ZeroVector;

	// Keeps the effect upright instead of turning with the socket, for flames and smoke.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	bool bWorldRotation = false;
};

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWTagEffect
{
	GENERATED_BODY()

	// Each one plays at its own sockets, so one tag can cover several body parts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	TArray<FPWTagEffectNiagara> Niagara;

	// Laid over every mesh of the actor while the tag is on. When several tags have one, the latest wins.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	TObjectPtr<UMaterialInterface> OverlayMaterial;
};

// What an actor shows while its ability system has a tag, such as flames while burning. Actors of one kind can share a set.
UCLASS(BlueprintType)
class PWGASCORE_API UPWTagEffectSet : public UDataAsset
{
	GENERATED_BODY()

public:
	// A parent tag also plays for its children: an entry for Status.Burn plays for Status.Burn.Strong.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects", meta=(ForceInlineRow))
	TMap<FGameplayTag, FPWTagEffect> Effects;
};
