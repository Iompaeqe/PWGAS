// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWActionModule.h"
#include "Ability/SpellParams.h"
#include "PWAbilityModule_Aura.generated.h"

struct FPWEffectEntry;
class USphereComponent;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Aura : public UPWActionModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated_Implementation() override;
	virtual void OnAbilityEnded_Implementation() override;
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> AuraSphere = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> OverlappingActors;

	// Object types the aura affects; anything else, projectiles included, passes through it.
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<ECollisionChannel>> ObjectChannels = {ECC_Pawn, ECC_WorldDynamic, ECC_PhysicsBody, ECC_Destructible};

	// Draws the aura's sphere when it starts.
	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// An actor counts once, however many of its components overlap.
	void EnterAura(AActor* Actor);

	// Applies or removes gameplay effects to overlapping actors
	void ApplyAuraEffect(const AActor* Target, bool bApply) const;
};
