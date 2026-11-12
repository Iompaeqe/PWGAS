// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "GameplayEffectTypes.h"
#include "FEffectProperties.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FEffectObject
{
	GENERATED_BODY()

	FEffectObject() {}
	FEffectObject(UAbilitySystemComponent* ASC, AActor* AA, AController* Controller, ACharacter* Char)
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
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {}
	
	FEffectObject Source;
	FEffectObject Target;
	FGameplayEffectContextHandle EffectContextHandle;
	FGameplayTagContainer EffectAssetTags;
};