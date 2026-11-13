// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "GameplayEffectTypes.h"
#include "PWEffectProperties.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FPWEffectObject
{
	GENERATED_BODY()

	FPWEffectObject() {}
	FPWEffectObject(UAbilitySystemComponent* ASC, AActor* AA, AController* Controller, ACharacter* Char)
	: ASC(ASC), AvatarActor(AA), Controller(Controller), Character(Char) {}

	UPROPERTY()
	bool bIsValid = false;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC = nullptr;
	UPROPERTY()
	TObjectPtr<AActor> AvatarActor = nullptr;
	UPROPERTY()
	TObjectPtr<AController> Controller = nullptr;
	UPROPERTY()
	TObjectPtr<ACharacter> Character = nullptr;
	UPROPERTY()
	FUniqueNetIdRepl PlayerNetId;
};

USTRUCT()
struct FPWEffectProperties
{
	GENERATED_BODY()

	FPWEffectProperties() {}
	
	FPWEffectObject Source;
	FPWEffectObject Target;
	FGameplayEffectContextHandle EffectContextHandle;
	FGameplayTagContainer EffectAssetTags;
};