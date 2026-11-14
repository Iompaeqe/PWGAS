// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "VisualAndAudio/PWMontageProvider.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

const UObject* IPWMontageProvider::Find(const AActor* Actor)
{
	if (!Actor)
		return nullptr;
	if (Actor->Implements<UPWMontageProvider>())
		return Actor;
	for (const UActorComponent* Component : Actor->GetComponents())
	{
		if (Component && Component->Implements<UPWMontageProvider>())
			return Component;
	}
	return nullptr;
}
