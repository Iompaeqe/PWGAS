// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "Precast/Data/PWPrecastInterfaces.h"
// #include "UObject/Object.h"
// #include "CircleDecalVisualizer.generated.h"
//
// /**
//  *	THIS IS AN EXAMPLE CLASS FOR THE PRECAST SYSTEMS' USAGE.
//  *	APrecastDecal Actor class is a basic actor that will show up to visualize the precast. Any type of visual setup can be used.
//  */
//
// struct FPWPrecastVisualUpdate;
//
// UCLASS()
// class PWGASCORE_API UCircleDecalVisualizer : public UPWPrecastVisualizer
// {
// 	GENERATED_BODY()
// public:
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<class APrecastDecal> DecalClass;
//
// 	virtual void Ensure_Implementation(APlayerController* PC, float Radius, FName) override
// 	{
// 		if (!Decal && DecalClass) Decal = PC->GetWorld()->SpawnActor<APrecastDecal>(DecalClass);
// 		if (Decal) { Decal->SetActorHiddenInGame(true); Decal->SetRadius(Radius); }
// 	}
// 	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override
// 	{
// 		if (!Decal) return;
// 		Decal->SetActorHiddenInGame(false);
// 		Decal->SetRadius(Data.Radius);
// 		Decal->SetActorLocation(Data.ImpactPoint);
// 	}
// 	virtual void Hide_Implementation() override
// 	{
// 		if (Decal) Decal->SetActorHiddenInGame(true);
// 	}
// private:
// 	UPROPERTY() APrecastDecal* Decal = nullptr;
// };