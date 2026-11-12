// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PWAbilityModule.generated.h"

class UPWDataModule;
class UPWModularGameplayAbility;
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Ability")
	TObjectPtr<UPWModularGameplayAbility> OwnerAbility = nullptr;

	UFUNCTION(BlueprintCallable, Category="Ability")
	virtual void Initialize(UPWModularGameplayAbility* InOwner);

	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void OnAbilityActivated();

	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void OnAbilityEnded();

	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void GetRequiredDataModules(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);

protected:
	APlayerController* GetPC() const;
	APawn* GetPawn() const;
};
