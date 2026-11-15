// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWAbilityValueCustomization.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GAS/Abilities/Values/PWAbilityValue.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PWAbilityValueCustomization"

namespace PWAbilityValueUI
{
	// Class metadata: the field edited right in the row, and the badge and label shown for values without one.
	const FName InlineProperty(TEXT("InlineProperty"));
	const FName ValueBadge(TEXT("ValueBadge"));
	const FName ValueLabel(TEXT("ValueLabel"));

	enum class EGroup : uint8 { Constant, Ability, Math, Custom, Num };
	constexpr int32 GroupCount = static_cast<int32>(EGroup::Num);

	struct FOption
	{
		FTopLevelAssetPath Path;
		FText Label;
		FText ToolTip;
	};

	const UPWAbilityValue* GetValue(const TSharedRef<IPropertyHandle>& Handle, bool& bMultiple)
	{
		UObject* Object = nullptr;
		bMultiple = Handle->GetValue(Object) == FPropertyAccess::MultipleValues;
		return Cast<UPWAbilityValue>(Object);
	}

	// Fields under a node, past category rows, which have no property.
	void CollectFields(const TSharedRef<IPropertyHandle>& Node, TArray<TSharedRef<IPropertyHandle>>& Out)
	{
		uint32 Num = 0;
		Node->GetNumChildren(Num);
		for (uint32 i = 0; i < Num; ++i)
		{
			const TSharedPtr<IPropertyHandle> Child = Node->GetChildHandle(i);
			if (!Child.IsValid())
				continue;
			if (Child->GetProperty())
				Out.Add(Child.ToSharedRef());
			else
				CollectFields(Child.ToSharedRef(), Out);
		}
	}

	// The value object's own fields. Its object row repeats the value's property, so the fields sit below that, never inside nested values.
	void GetOwnFields(const TSharedRef<IPropertyHandle>& Handle, TArray<TSharedRef<IPropertyHandle>>& Out)
	{
		uint32 Num = 0;
		Handle->GetNumChildren(Num);
		for (uint32 i = 0; i < Num; ++i)
		{
			const TSharedPtr<IPropertyHandle> Child = Handle->GetChildHandle(i);
			if (!Child.IsValid())
				continue;
			if (!Child->GetProperty() || Child->GetProperty() == Handle->GetProperty())
				CollectFields(Child.ToSharedRef(), Out);
			else
				Out.Add(Child.ToSharedRef());
		}
	}

	EGroup GetGroup(const UClass* Class)
	{
		if (!Class)
			return EGroup::Custom;
		if (Class->IsChildOf(UPWAbilityValue_Constant::StaticClass()))
			return EGroup::Constant;
		if (Class->IsChildOf(UPWAbilityValue_Math::StaticClass()))
			return EGroup::Math;
		return Class->HasMetaData(ValueBadge) ? EGroup::Ability : EGroup::Custom;
	}

	// Every kind of value, Blueprint ones that aren't loaded yet included.
	void GatherOptions(TArray<FOption> (&Groups)[GroupCount])
	{
		const FTopLevelAssetPath Base = UPWAbilityValue::StaticClass()->GetClassPathName();
		TSet<FTopLevelAssetPath> Paths;
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetDerivedClassNames({Base}, {}, Paths);
		TArray<UClass*> Loaded;
		GetDerivedClasses(UPWAbilityValue::StaticClass(), Loaded);
		for (const UClass* Class : Loaded)
			Paths.Add(Class->GetClassPathName());

		for (const FTopLevelAssetPath& Path : Paths)
		{
			const FString Name = Path.GetAssetName().ToString();
			if (Path == Base || Name.StartsWith(TEXT("SKEL_")) || Name.StartsWith(TEXT("REINST_")))
				continue;
			const UClass* Class = FindObject<UClass>(Path);
			if (Class && Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Hidden | CLASS_HideDropDown))
				continue;

			FOption Option;
			Option.Path = Path;
			Option.Label = Class ? FBlueprintEditorUtils::GetFriendlyClassDisplayName(Class) : FText::FromString(Name.EndsWith(TEXT("_C")) ? Name.LeftChop(2) : Name);
			Option.ToolTip = Class ? Class->GetToolTipText() : FText::GetEmpty();
			Groups[static_cast<int32>(GetGroup(Class))].Add(MoveTemp(Option));
		}
		for (TArray<FOption>& Group : Groups)
			Group.Sort([](const FOption& L, const FOption& R) { return L.Label.CompareTo(R.Label) < 0; });
	}

	void Pick(const TSharedRef<IPropertyHandle>& Handle, const FTopLevelAssetPath& Path, const TSharedPtr<IPropertyUtilities>& Utilities)
	{
		UClass* Class = nullptr;
		if (Path.IsValid())
		{
			Class = LoadObject<UClass>(nullptr, *Path.ToString());
			if (!Class)
				return;
		}
		PropertyCustomizationHelpers::CreateNewInstanceOfEditInlineObjectClass(Handle, Class);
		if (Utilities.IsValid())
			Utilities->ForceRefresh();
	}

	TSharedRef<SWidget> MakeMenu(const TSharedRef<IPropertyHandle>& Handle, const TSharedPtr<IPropertyUtilities>& Utilities)
	{
		TArray<FOption> Groups[GroupCount];
		GatherOptions(Groups);
		const FText Headings[GroupCount] = { LOCTEXT("Constant", "Constant"), LOCTEXT("Ability", "Ability"), LOCTEXT("Math", "Math"), LOCTEXT("Custom", "Custom") };

		FMenuBuilder Menu(true, nullptr);
		for (int32 i = 0; i < GroupCount; ++i)
		{
			if (Groups[i].IsEmpty())
				continue;
			Menu.BeginSection(NAME_None, Headings[i]);
			for (const FOption& Option : Groups[i])
			{
				const FTopLevelAssetPath Path = Option.Path;
				Menu.AddMenuEntry(Option.Label, Option.ToolTip, FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([Handle, Path, Utilities]() { Pick(Handle, Path, Utilities); })));
			}
			Menu.EndSection();
		}
		Menu.BeginSection(NAME_None);
		Menu.AddMenuEntry(LOCTEXT("Clear", "Clear"), LOCTEXT("ClearTip", "Leaves it unset, which reads 0."), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([Handle, Utilities]() { Pick(Handle, FTopLevelAssetPath(), Utilities); })));
		Menu.EndSection();
		return Menu.MakeWidget();
	}

	FLinearColor GetBadgeColor(const FString& Badge)
	{
		if (Badge == TEXT("MODULE"))
			return FLinearColor(0.02f, 0.22f, 0.5f);
		if (Badge == TEXT("ABILITY"))
			return FLinearColor(0.08f, 0.35f, 0.12f);
		if (Badge == TEXT("VAR"))
			return FLinearColor(0.38f, 0.1f, 0.45f);
		return FLinearColor(0.2f, 0.2f, 0.2f);
	}

	TSharedRef<SWidget> MakeBadge(const FString& Badge)
	{
		static const FSlateRoundedBoxBrush Brush(FLinearColor::White, 4.f);
		return SNew(SBorder)
			.BorderImage(&Brush)
			.BorderBackgroundColor(GetBadgeColor(Badge))
			.Padding(FMargin(6.f, 1.f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Badge))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
				.ColorAndOpacity(FLinearColor::White)
			];
	}

	TSharedRef<SWidget> MakeText(const FText& Text, const bool bSubdued = false)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(bSubdued ? FSlateColor::UseSubduedForeground() : FSlateColor::UseForeground());
	}

	// What shows in the row itself: the inline field, or the badge and label, or None.
	TSharedRef<SWidget> MakeInline(const TSharedRef<IPropertyHandle>& Handle, const UPWAbilityValue* Value, const bool bMultiple)
	{
		if (bMultiple)
			return MakeText(LOCTEXT("Multiple", "Multiple Values"), true);
		if (!Value)
			return MakeText(LOCTEXT("None", "None"), true);

		const UClass* Class = Value->GetClass();
		const TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox).ToolTipText(Class->GetToolTipText());
		const FString& Badge = Class->GetMetaData(ValueBadge);
		if (!Badge.IsEmpty())
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
			[
				MakeBadge(Badge)
			];
		}

		TSharedPtr<IPropertyHandle> Field;
		if (Class->HasMetaData(InlineProperty))
		{
			const FName FieldName(*Class->GetMetaData(InlineProperty));
			TArray<TSharedRef<IPropertyHandle>> Fields;
			GetOwnFields(Handle, Fields);
			for (const TSharedRef<IPropertyHandle>& Each : Fields)
			{
				if (Each->GetProperty()->GetFName() == FieldName)
					Field = Each;
			}
		}

		if (Field.IsValid())
		{
			Row->AddSlot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				Field->CreatePropertyValueWidget(false)
			];
		}
		else
		{
			const FString& Label = Class->GetMetaData(ValueLabel);
			Row->AddSlot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				MakeText(Label.IsEmpty() ? FBlueprintEditorUtils::GetFriendlyClassDisplayName(Class) : FText::FromString(Label))
			];
		}
		return Row;
	}
}

TSharedRef<IPropertyTypeCustomization> FPWAbilityValueCustomization::MakeInstance()
{
	return MakeShareable(new FPWAbilityValueCustomization);
}

void FPWAbilityValueCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& Utils)
{
	using namespace PWAbilityValueUI;

	bool bMultiple = false;
	const UPWAbilityValue* Value = GetValue(PropertyHandle, bMultiple);
	const TSharedPtr<IPropertyUtilities> Utilities = Utils.GetPropertyUtilities();

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.f)
	.MaxDesiredWidth(600.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			MakeInline(PropertyHandle, Value, bMultiple)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 0.f, 0.f, 0.f)
		[
			SNew(SComboButton)
			.ComboButtonStyle(&FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("SimpleComboButton"))
			.HasDownArrow(true)
			.ToolTipText(LOCTEXT("PickTip", "Pick what this value reads: a constant, a module's value, the ability level, a variable of the ability, or math on those."))
			.OnGetMenuContent_Lambda([PropertyHandle, Utilities]() { return MakeMenu(PropertyHandle, Utilities); })
		]
	];
}

void FPWAbilityValueCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& Utils)
{
	using namespace PWAbilityValueUI;

	bool bMultiple = false;
	const UPWAbilityValue* Value = GetValue(PropertyHandle, bMultiple);
	if (!Value || bMultiple || Value->GetClass()->HasMetaData(InlineProperty))
		return;

	TArray<TSharedRef<IPropertyHandle>> Fields;
	GetOwnFields(PropertyHandle, Fields);
	for (const TSharedRef<IPropertyHandle>& Field : Fields)
		ChildBuilder.AddProperty(Field);
}

#undef LOCTEXT_NAMESPACE
