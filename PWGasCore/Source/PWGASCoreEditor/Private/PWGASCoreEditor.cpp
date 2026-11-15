// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWGASCoreEditor.h"

#include "PWAbilitySetEntryCustomization.h"
#include "PWAbilityValueCustomization.h"
#include "PWGameplayAbility_DetailCustomization.h"

#define LOCTEXT_NAMESPACE "FPWGASCoreEditorModule"

void FPWGASCoreEditorModule::StartupModule()
{
	FPropertyEditorModule& Prop = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	Prop.RegisterCustomClassLayout(TEXT("PWGameplayAbilityBase"),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPWGameplayAbility_DetailCustomization::MakeInstance));
	Prop.RegisterCustomPropertyTypeLayout(TEXT("PWAbilitySetEntry"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPWAbilitySetEntryCustomization::MakeInstance));
	Prop.RegisterCustomPropertyTypeLayout(TEXT("PWAbilityValue"),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPWAbilityValueCustomization::MakeInstance));
}

void FPWGASCoreEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPWGASCoreEditorModule, PWGASCoreEditor)