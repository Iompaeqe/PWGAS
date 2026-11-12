// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "NativeGameplayTags.h"


namespace PWTags::Input
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GlobalCancel);
}

namespace PWTags::Event
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input);
}

namespace PWTags::ASC
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
}

namespace PWTags::MontageEvent
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trigger);
}

namespace PWTags::Ability
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
}

namespace PWTags::Ability::SetByCaller
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown);
}

namespace PWTags::Ability::Activation
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Failed);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnGranted);
}

namespace PWTags::Ability::Deactivation
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnRemoved);
}

namespace PWTags::Ability::State
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AwaitingConfirmation);
}

namespace PWTags::Ability::Skill
{
	PWGASCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Root);

	inline FGameplayTag GetCooldownTag(const FGameplayTag& AbilityTag)
	{
		return UGameplayTagsManager::Get().RequestGameplayTag(
			*FString::Printf(TEXT("%s.Cooldown"), *AbilityTag.GetTagName().ToString()), false
		);
	}

	inline FString GetCooldownSuffix()
	{
		return "Cooldown";
	}
}

