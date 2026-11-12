// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"

struct FPWTestWorld
{
	UWorld* World = nullptr;
	AActor* Actor = nullptr;
	UPWAbilitySystemComponent* ASC = nullptr;

	FPWTestWorld()
	{
		World = UWorld::CreateWorld(EWorldType::Game, false);
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();

		ASC = SpawnASC();
		Actor = ASC->GetOwner();
	}

	~FPWTestWorld()
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}

	UPWAbilitySystemComponent* SpawnASC() const
	{
		APawn* NewActor = World->SpawnActor<APawn>();
		USceneComponent* Root = NewObject<USceneComponent>(NewActor);
		NewActor->SetRootComponent(Root);
		Root->RegisterComponent();
		UPWAbilitySystemComponent* NewASC = NewObject<UPWAbilitySystemComponent>(NewActor);
		NewASC->RegisterComponent();
		NewASC->InitAbilityActorInfo(NewActor, NewActor);
		return NewASC;
	}

	template <typename T>
	T* ActivateAbility() const
	{
		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(T::StaticClass(), 1));
		ASC->TryActivateAbility(Handle);
		return Cast<T>(ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	}

	template <typename T>
	TArray<T*> GetActors() const
	{
		TArray<T*> Actors;
		for (TActorIterator<T> It(World); It; ++It)
			Actors.Add(*It);
		return Actors;
	}

	void TickTimers(const float DeltaTime) const
	{
		GFrameCounter++;
		World->GetTimerManager().Tick(DeltaTime);
	}
};
