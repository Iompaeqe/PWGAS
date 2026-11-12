// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Libraries/PWAbilityMathLibrary.h"


TArray<FRotator> UPWAbilityMathLibrary::EvenlySpreadRotators(const FVector& Forward, const FVector& Axis, const float Spread, const int32 Count)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

	if (Count > 1)
	{
		const float DeltaSpread = Spread / (Count - 1);
		for (int i = 0; i < Count; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	
	return Rotators;
}

TArray<FVector> UPWAbilityMathLibrary::EvenlySpreadVectors(const FVector& Forward, const FVector& Axis, const float Spread, const int32 Count)
{
	TArray<FVector> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

	if (Count > 1)
	{
		const float DeltaSpread = Spread / (Count - 1);
		for (int i = 0; i < Count; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Rotators.Add(Direction);
		}
	}
	else
	{
		Rotators.Add(Forward);
	}
	
	return Rotators;
}

void UPWAbilityMathLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}
	
	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while(NumTargetsFound < MaxTargets)
	{
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor;
		for (AActor* PotentialTarget : ActorsToCheck)
		{
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		NumTargetsFound++;
	}
}

void UPWAbilityMathLibrary::GetClosestTarget(const TArray<AActor*>& Actors, AActor*& OutClosestTarget, const FVector& Origin)
{
	TArray<AActor*> OutClosestTargets;
	
	if (Actors.Num() <= 0)
	{
		OutClosestTargets = Actors;
		return;
	}
	
	GetClosestTargets(1, Actors, OutClosestTargets, Origin);
	
	OutClosestTarget = OutClosestTargets[0];
}
