// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisualAndAudio/PWMontageProvider.h"
#include "PWMontageSetComponent.generated.h"

class UPWMontageSet;

// Gives any character or pawn the montages its abilities ask for.
UCLASS(ClassGroup=(GASCore), Blueprintable, meta=(BlueprintSpawnableComponent, DisplayName="PW Montage Set"))
class PWGASCORE_API UPWMontageSetComponent : public UActorComponent, public IPWMontageProvider
{
	GENERATED_BODY()

public:
	UPWMontageSetComponent();

	UFUNCTION(BlueprintPure, Category="Montages")
	UPWMontageSet* GetMontageSet() const { return MontageSet; }

	// Server. Swaps the montages, for example when a weapon changes the moves.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Montages")
	void SetMontageSet(UPWMontageSet* NewSet);

	//~IPWMontageProvider
	virtual UAnimMontage* GetMontageForTag_Implementation(FGameplayTag AnimTag, float& OutAnimRate) const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category="Montages")
	TObjectPtr<UPWMontageSet> MontageSet;
};
