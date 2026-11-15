// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWActionModule.h"
#include "Ability/PWSpellParams.h"
#include "Engine/TimerHandle.h"
#include "PWAbilityModule_Aura.generated.h"

struct FPWEffectEntry;
class USphereComponent;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="PWM Aura"))
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
	UPROPERTY(EditAnywhere, Category="Aura")
	TArray<TEnumAsByte<ECollisionChannel>> ObjectChannels = {ECC_Pawn, ECC_WorldDynamic, ECC_PhysicsBody, ECC_Destructible};

	FTimerHandle DebugTimer;

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

	// Redraws the aura where the caster is now, on a timer while Show Debug is on.
	void DrawDebugAura();
};
