// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PWAbilityModule.generated.h"

class UPWDataModule;
class UPWModularGameplayAbility;

// One colour per kind of thing the modules draw, so shapes from different modules read the same way.
namespace PWModuleDebug
{
	inline const FColor Radius = FColor::Orange;
	inline const FColor Range = FColor(60, 140, 255);
	inline const FColor Target = FColor::Red;
	inline const FColor Position = FColor::Green;
	inline const FColor Direction = FColor::Yellow;
	inline const FColor Text = FColor::White;
	// Seconds a one-off drawing or message stays up.
	constexpr float Duration = 3.f;
}

// Moments of a charge or channel that modules hear about, besides activation and end.
UENUM(BlueprintType)
enum class EPWAbilityPhase : uint8
{
	// The charge or channel started.
	Started,
	// One channel or charge tick passed.
	Pulse,
	// The charge was released or the channel finished.
	Completed,
	// The charge or channel was cancelled.
	Cancelled
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Ability")
	TObjectPtr<UPWModularGameplayAbility> OwnerAbility = nullptr;

	// Draws what this module does in the world and prints it on screen and in the log, to line its numbers up with the visual effects. Shipping builds leave it out.
	// Graphs read it with Is Debugging. Not Blueprint visible, so the details panel lists Debug after the module's own fields.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bShowDebug = false;

	UFUNCTION(BlueprintCallable, Category="Ability")
	virtual void Initialize(UPWModularGameplayAbility* InOwner);

	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void OnAbilityActivated();

	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void OnAbilityEnded();

	// Started and Pulse come after the ability's own charge or channel event, so they see what its graph set, and only while it still runs.
	// Completed and Cancelled come before it, while the ability still runs, since that event usually ends it.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void OnAbilityPhase(EPWAbilityPhase Phase);

	// Drops the values the last cast set on this module, so its configured ones apply again. Called as the ability activates, not as it ends,
	// so a cast keeps its values through spawns that come after it ends.
	virtual void ResetCurrentValues() {}

	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void GetRequiredDataModules(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules);

	// Whether Show Debug is on and this build can draw.
	UFUNCTION(BlueprintPure, Category="Debug")
	bool IsDebugging() const;

	// Prints on screen and in the log, starting with the world (Server, Client 1), the ability and the module. Does nothing unless Show Debug is on.
	// A Key from Debug Key keeps one screen line that each call replaces, for things printed every frame; those skip the log.
	UFUNCTION(BlueprintCallable, Category="Debug")
	void DebugPrint(const FString& Message, FLinearColor Color = FLinearColor(0.f, 0.66f, 1.f, 1.f), float Duration = 3.f, int32 Key = -1) const;

	// A screen line key unique to this module; Slot tells several lines of one module apart.
	UFUNCTION(BlueprintPure, Category="Debug")
	int32 DebugKey(int32 Slot = 0) const;

	// Drawing helpers for this module and the ones that use its values; each does nothing unless Show Debug is on.
	void DebugSphere(const FVector& Center, float Radius, const FColor& Color, float Duration = PWModuleDebug::Duration) const;
	// A flat circle around Center, for ranges and areas on the ground.
	void DebugCircle(const FVector& Center, float Radius, const FColor& Color, float Duration = PWModuleDebug::Duration) const;
	void DebugLine(const FVector& From, const FVector& To, const FColor& Color, float Duration = PWModuleDebug::Duration) const;
	void DebugArrow(const FVector& From, const FVector& To, const FColor& Color, float Duration = PWModuleDebug::Duration) const;
	void DebugPoint(const FVector& Location, const FColor& Color, float Duration = PWModuleDebug::Duration) const;
	// Red, green and blue lines along the transform's forward, right and up.
	void DebugAxes(const FTransform& Transform, float Size = 50.f, float Duration = PWModuleDebug::Duration) const;
	void DebugText(const FVector& Location, const FString& Text, const FColor& Color = PWModuleDebug::Text, float Duration = PWModuleDebug::Duration) const;

protected:
	APlayerController* GetPC() const;
	APawn* GetPawn() const;
	UWorld* GetDebugWorld() const;
	// The owner ability's level, 1 without one.
	int32 GetOwnerLevel() const;
};
