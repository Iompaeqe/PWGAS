// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Ability/Affectable.h"
#include "Ability/EffectReceiver.h"
#include "GameFramework/Actor.h"
#include "PWDamageableActor.generated.h"

class UPWDamageableComponent;

// Ready-made actor that takes ability effects through a damageable component. Any actor can do the same by adding the component and forwarding ApplyEffects to it.
UCLASS(Blueprintable)
class PWGASCORE_API APWDamageableActor : public AActor, public IEffectReceiver, public IAffectable
{
	GENERATED_BODY()

public:
	APWDamageableActor();

	UFUNCTION(BlueprintPure, Category="Damageable")
	UPWDamageableComponent* GetDamageable() const { return Damageable; }

protected:
	virtual bool ApplyEffects_Implementation(USpellParams* SpellParams) override;
	virtual void RemoveEffects_Implementation(USpellParams* SpellParams, bool bInfiniteOnly) override;
	virtual UObject* GetEffectReceiver_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Damageable")
	TObjectPtr<UPWDamageableComponent> Damageable;
};
