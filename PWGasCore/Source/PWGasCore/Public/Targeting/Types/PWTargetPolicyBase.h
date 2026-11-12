// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWTargetPolicyBase.generated.h"

class UPWDataModule;
class UPWModularGameplayAbility;
struct FPWTargetingResult;

// UINTERFACE(BlueprintType)
// class UPWTargetingSource : public UInterface { GENERATED_BODY() };
// class IPWTargetingSource
// {
// 	GENERATED_BODY()
// public:
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
// 	bool Sample(class APlayerController* PC, FPWTargetingResult& Out);
// };
//
// UINTERFACE(BlueprintType)
// class UPWRangePolicy : public UInterface { GENERATED_BODY() };
// class IPWRangePolicy
// {
// 	GENERATED_BODY()
// public:
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
// 	void Apply(const class AActor* Origin, float MaxRange, FPWTargetingResult& InOut);
// };
//
// UINTERFACE(BlueprintType)
// class UPWTargetResolver : public UInterface { GENERATED_BODY() };
//
// class IPWTargetResolver
// {
// 	GENERATED_BODY()
// public:
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
// 	void Resolve(APlayerController* PC, const FPWTargetingResult& In, const float Radius, TArray<AActor*>& OutActors) const;
// };


/**
 * Base class for all targeting-related policy objects.
 * Policies are instanced and owned by Gameplay Abilities.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWTargetPolicyBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPWModularGameplayAbility> OwnerAbility = nullptr;
	
	/**
	 * Initialize any custom variables here.
	 * @param InOwner Owning Gameplay Ability
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	void Initialize(UPWModularGameplayAbility* InOwner);
	virtual void Initialize_Implementation(UPWModularGameplayAbility* InOwner);

	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void GetRequiredDataModules(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);
};

/**
 * Provides the source (location or direction) of targeting.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWTargetingSource : public UPWTargetPolicyBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	bool Sample(APlayerController* PC, FPWTargetingResult& Out) const;
	virtual bool Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	bool SampleFrom(const APawn* Pawn, const FVector& Origin, FPWTargetingResult& Out) const;
	virtual bool SampleFrom_Implementation(const APawn* Pawn, const FVector& Origin, FPWTargetingResult& Out) const;
};

/**
 * Range-limiting logic for targeting results.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWRangePolicy : public UPWTargetPolicyBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	void Apply(const AActor* Actor, float MaxRange, FPWTargetingResult& InOut) const;
	virtual void Apply_Implementation(const AActor* Actor, float MaxRange, FPWTargetingResult& InOut) const;
};

/**
 * Resolves targeting into concrete actors.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWTargetResolver : public UPWTargetPolicyBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	void Resolve(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const;
	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const;
};

