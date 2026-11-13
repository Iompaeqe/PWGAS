// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "Components/ActorComponent.h"
#include "PWDamageableComponent.generated.h"

class USpellParams;
class UPWDamageableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPWDamageableHealthChanged, UPWDamageableComponent*, Component, float, OldHealth, float, NewHealth, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPWDamageableDepleted, UPWDamageableComponent*, Component, AActor*, Instigator);

// Health for actors that take ability effects without an ability system component: crates, barricades, destructible props.
UCLASS(ClassGroup=(GASCore), Blueprintable, meta=(BlueprintSpawnableComponent))
class PWGASCORE_API UPWDamageableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPWDamageableComponent();

	// Effects that change this attribute are read as damage, usually the game's incoming damage attribute.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damageable")
	FGameplayAttribute DamageAttribute;

	// Health the actor starts with and cannot be healed above.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damageable", meta=(ClampMin="0"))
	float MaxHealth = 100.f;

	// Destroys the owning actor when health reaches zero.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damageable")
	bool bDestroyOwnerWhenDepleted = true;

	// Server and clients.
	UPROPERTY(BlueprintAssignable, Category="Damageable")
	FPWDamageableHealthChanged OnHealthChanged;

	// Server only.
	UPROPERTY(BlueprintAssignable, Category="Damageable")
	FPWDamageableDepleted OnDepleted;

	// Reads the effects carried by the spell params. Instant ones hit once, periodic ones keep ticking.
	UFUNCTION(BlueprintCallable, Category="Damageable")
	bool ApplySpellParams(USpellParams* SpellParams);

	// A negative amount heals.
	UFUNCTION(BlueprintCallable, Category="Damageable")
	void ApplyDamage(float Amount, AActor* Instigator);

	// Ends the ongoing effects this caster applied with these spell params.
	UFUNCTION(BlueprintCallable, Category="Damageable")
	void RemoveSpellParams(const USpellParams* SpellParams, bool bInfiniteOnly = false);

	UFUNCTION(BlueprintCallable, Category="Damageable")
	void ClearOngoingEffects();

	UFUNCTION(BlueprintCallable, Category="Damageable")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category="Damageable")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Damageable")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category="Damageable")
	bool IsDepleted() const { return bDepleted; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Lets a game scale or reject damage, for example armor or immunity.
	UFUNCTION(BlueprintNativeEvent, Category="Damageable")
	float ModifyDamage(float Amount, AActor* Instigator) const;

	UFUNCTION(BlueprintNativeEvent, Category="Damageable")
	void HandleDepleted(AActor* Instigator);

	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category="Damageable")
	float Health = 0.f;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

private:
	struct FPWOngoingEffect
	{
		FTimerHandle Tick;
		FTimerHandle End;
		TWeakObjectPtr<AActor> Instigator;
		TSubclassOf<UGameplayEffect> EffectClass;
		bool bInfinite = false;
	};

	void TickOngoing(float Amount, TWeakObjectPtr<AActor> Instigator);
	void EndOngoing(FTimerHandle TickHandle);
	void StopOngoing(const FPWOngoingEffect& Effect) const;
	bool HasAuthority() const;

	TArray<FPWOngoingEffect> OngoingEffects;
	bool bDepleted = false;
};
