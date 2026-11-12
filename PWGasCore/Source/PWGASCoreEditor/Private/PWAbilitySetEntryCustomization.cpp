// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWAbilitySetEntryCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "GAS/Abilities/PWAbilityInfo.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "Settings/PWGASCoreSettings.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FPWAbilitySetEntryCustomization::MakeInstance()
{
	return MakeShareable(new FPWAbilitySetEntryCustomization);
}

void FPWAbilitySetEntryCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& Utils)
{
	const TSharedPtr<IPropertyHandle> TagHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPWAbilitySetEntry, AbilityTag));

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(400.f)
	[
		SNew(STextBlock)
		.Text_Lambda([TagHandle]()
		{
			const FString Registry = UPWGASCoreSettings::Get()->GetAbilityRegistry(FGameplayTag()).GetAssetName();
			return FText::FromString(FString::Printf(TEXT("Ability Tag [%s] is not in %s"), *GetTag(TagHandle).ToString(), Registry.IsEmpty() ? TEXT("the ability registry") : *Registry));
		})
		.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		.ColorAndOpacity(FLinearColor(1.f, 0.35f, 0.2f))
		.Visibility_Lambda([TagHandle]()
		{
			return IsRegistered(GetTag(TagHandle)) ? EVisibility::Collapsed : EVisibility::Visible;
		})
	];
}

void FPWAbilitySetEntryCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& Utils)
{
	uint32 NumChildren = 0;
	PropertyHandle->GetNumChildren(NumChildren);
	for (uint32 Index = 0; Index < NumChildren; ++Index)
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
}

FGameplayTag FPWAbilitySetEntryCustomization::GetTag(const TSharedPtr<IPropertyHandle>& TagHandle)
{
	void* Data = nullptr;
	if (!TagHandle.IsValid() || TagHandle->GetValueData(Data) != FPropertyAccess::Success || !Data)
		return FGameplayTag();

	return *static_cast<const FGameplayTag*>(Data);
}

bool FPWAbilitySetEntryCustomization::IsRegistered(const FGameplayTag& Tag)
{
	return !Tag.IsValid() || UPWGASCoreSettings::Get()->GetAbilityByTag(Tag).IsValid();
}
