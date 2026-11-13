// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Modules/PWAbilityModule.h"
#include "PWActionModule.generated.h"

class UPWDataModule;
class UPWSpellParams;
struct FPWTargetingResult;
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWActionModule : public UPWAbilityModule
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category="Module")
	UPWSpellParams* MakeSpellParams(const FPWTargetingResult& Target);
	virtual UPWSpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target);

	virtual void ExtendParamsWithData(UPWSpellParams* Params) const;

	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void ExecuteAction() {}

	virtual void GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const {}
	virtual TSubclassOf<AActor> GetPreviewActorClass() const { return nullptr; }
	virtual void GetPreviewAimPoints(const FPWTargetingResult& Target, TArray<FVector>& OutAimPoints) const {}

	FSimpleMulticastDelegate OnActionFinished;
	bool IsActionPending() const { return bActionPending; }

protected:
	void StartAction();
	void FinishAction();

private:
	bool bActionPending = false;
};
