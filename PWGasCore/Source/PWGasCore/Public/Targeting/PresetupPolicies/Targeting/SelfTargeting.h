// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "SelfTargeting.generated.h"

UCLASS()
class PWGASCORE_API USelfTargeting : public UPWTargetingSource
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting")
	FVector Offset = FVector::ZeroVector;

	virtual bool Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const override
	{
		const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		return Pawn && SampleFrom(Pawn, Pawn->GetActorLocation(), Out);
	}

	virtual bool SampleFrom_Implementation(const APawn* Pawn, const FVector& Origin, FPWTargetingResult& Out) const override
	{
		if (!Pawn)
			return false;

		Out.Location = Origin + Pawn->GetActorRotation().RotateVector(Offset);
		Out.Rotation = Pawn->GetActorRotation();
		Out.Hit.Location = Out.Location;
		Out.Hit.ImpactPoint = Out.Location;
		return true;
	}
};
