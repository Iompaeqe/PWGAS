// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"

namespace PWTags::Attributes {
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute);
}


class FPWAttributesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
