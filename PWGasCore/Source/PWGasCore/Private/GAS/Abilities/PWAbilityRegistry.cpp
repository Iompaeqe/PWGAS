// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/PWAbilityRegistry.h"

#include "GameplayAbilitySpec.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GAS/Abilities/PWAbilityDataAsset.h"
#include "GAS/Abilities/PWAbilityInfo.h"
#include "GAS/Abilities/PWAbilityRegistryProvider.h"
#include "Settings/PWGASCoreSettings.h"
#if WITH_EDITOR
#include "AssetRegistry/IAssetRegistry.h"
#endif
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "Misc/DataValidation.h"

struct FDescriptionNamedArguments
{
	FString _Level0 = FString::Printf(TEXT("_Level0"));
	FString _Level1 = FString::Printf(TEXT("_Level1"));
	FString _CD0 = FString::Printf(TEXT("_CD0"));
	FString _CD1 = FString::Printf(TEXT("_CD1"));
	FString _Cost0 = FString::Printf(TEXT("_Cost0"));
	FString _Cost1 = FString::Printf(TEXT("_Cost1"));
};

UPWAbilityRegistry* UPWAbilityRegistry::Get(const UObject* WorldContextObject)
{
	// Clients have no game mode, but the game state carries its class, so its defaults are readable everywhere.
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	const AGameModeBase* GameMode = GameState ? GameState->GetDefaultGameMode() : World ? World->GetAuthGameMode() : nullptr;
	if (GameMode && GameMode->Implements<UPWAbilityRegistryProvider>())
		if (UPWAbilityRegistry* Registry = IPWAbilityRegistryProvider::Execute_GetAbilityRegistry(GameMode))
			return Registry;

	return UPWGASCoreSettings::Get()->GetAbilityRegistry(FGameplayTag()).LoadSynchronous();
}

FPWAbilityInfo UPWAbilityRegistry::FindAbility(const UObject* WorldContextObject, const FGameplayTag AbilityTag)
{
	UPWAbilityRegistry* Registry = Get(WorldContextObject);
	return Registry ? Registry->GetAbilityByTag(AbilityTag) : FPWAbilityInfo();
}

FPWAbilityInfo UPWAbilityRegistry::GetAbilityByTag(const FGameplayTag& AbilityTag)
{
	const UPWAbilityDataAsset* Data = FindData(AbilityTag);
	return Data ? Data->Ability : FPWAbilityInfo();
}

void UPWAbilityRegistry::Preload()
{
	Cache.Reset();
	for (const TSoftObjectPtr<UPWAbilityDataAsset>& SoftAbilityDA : Abilities)
		if (UPWAbilityDataAsset* AbilityDA = SoftAbilityDA.LoadSynchronous())
			if (AbilityDA->Ability.Tag.IsValid() && !Cache.Contains(AbilityDA->Ability.Tag))
				Cache.Add(AbilityDA->Ability.Tag, AbilityDA);
	bCached = true;
}

UPWAbilityDataAsset* UPWAbilityRegistry::FindData(const FGameplayTag& AbilityTag)
{
	if (!bCached)
		Preload();

	const TObjectPtr<UPWAbilityDataAsset>* Found = Cache.Find(AbilityTag);
#if WITH_EDITOR
	// An ability's data asset can be edited while the cache is up, so a miss or a stale hit looks again.
	if (!Found || !*Found || !(*Found)->Ability.Tag.MatchesTagExact(AbilityTag))
	{
		Preload();
		Found = Cache.Find(AbilityTag);
	}
#endif
	return Found ? Found->Get() : nullptr;
}

#if WITH_EDITOR
bool UPWAbilityRegistry::IsInAnyRegistry(const FGameplayTag& AbilityTag)
{
	TArray<FAssetData> RegistryAssets;
	IAssetRegistry::GetChecked().GetAssetsByClass(StaticClass()->GetClassPathName(), RegistryAssets, true);
	for (const FAssetData& Asset : RegistryAssets)
		if (UPWAbilityRegistry* Registry = Cast<UPWAbilityRegistry>(Asset.GetAsset()))
			if (Registry->FindData(AbilityTag))
				return true;
	return false;
}

EDataValidationResult UPWAbilityRegistry::IsDataValid(FDataValidationContext& Context) const
{
	TMap<FGameplayTag, FString> SeenTags;
	for (const TSoftObjectPtr<UPWAbilityDataAsset>& SoftAbilityDA : Abilities)
	{
		const UPWAbilityDataAsset* AbilityDA = SoftAbilityDA.LoadSynchronous();
		if (!AbilityDA)
		{
			Context.AddError(FText::FromString(TEXT("Registry has an empty entry.")));
			continue;
		}

		if (const FString* Other = SeenTags.Find(AbilityDA->Ability.Tag))
			Context.AddError(FText::FromString(FString::Printf(TEXT("%s and %s use the same tag [%s]."), **Other, *AbilityDA->GetName(), *AbilityDA->Ability.Tag.ToString())));
		SeenTags.Add(AbilityDA->Ability.Tag, AbilityDA->GetName());

		if (AbilityDA->IsDataValid(Context) == EDataValidationResult::Invalid)
			Context.AddError(FText::FromString(FString::Printf(TEXT("%s is not valid."), *AbilityDA->GetName())));
	}

	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

void UPWAbilityRegistry::PostEditChangeProperty(FPropertyChangedEvent& E)
{
	Super::PostEditChangeProperty(E);
	Cache.Reset();
	bCached = false;
}
#endif

void UPWAbilityRegistry::MakeDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription)
{
	if (!AbilityTag.IsValid())
		OutDescription = FText();

	if (FGameplayAbilitySpec* Spec = ASC->Data().GetSpecOfAbility(AbilityTag))
	{
		OutDescription = GetDescriptionForAbilityTag(AbilityTag, Spec->Level);
		return;
	}
	else
	{
		OutDescription = GetDescriptionForAbilityTag(AbilityTag, 1);
	}
}

void UPWAbilityRegistry::MakeNextDescriptionsUsingAbilityTag(const FGameplayTag& AbilityTag, UPWAbilitySystemComponent* ASC, FText& OutDescription)
{
	if (!AbilityTag.IsValid())
		OutDescription = FText();

	if (FGameplayAbilitySpec* Spec = ASC->Data().GetSpecOfAbility(AbilityTag))
	{
		OutDescription = GetNextDescriptionForAbilityTag(AbilityTag, Spec->Level);
		return;
	}
	else
	{
		OutDescription = GetNextDescriptionForAbilityTag(AbilityTag, 1);
	}
}

FText UPWAbilityRegistry::GetDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level)
{
	FPWAbilityInfo Ability = GetAbilityByTag(AbilityTag);
	if (Ability.IsValid())
	{
		if (!Ability.Description.IsEmpty())
		{
			return FormatTextValues(AbilityTag, Level, Ability.Description);
		}
	}

	FText FormattedText = FText::Format(
		FText::FromString(TEXT("<Body>{0} - {1}, </><Level>{2}</>")),
		FText::FromString(AbilityTag.ToString()),
		FText::FromString(TEXT("Ability has no description, Ability Level:")),
		FText::AsNumber(Level)
	);


	return FormattedText;
}

FText UPWAbilityRegistry::GetNextDescriptionForAbilityTag(const FGameplayTag& AbilityTag, int32 Level)
{
	FPWAbilityInfo Ability = GetAbilityByTag(AbilityTag);
	if (Ability.IsValid())
	{
		if (!Ability.NextDescription.IsEmpty())
		{
			return FormatTextValues(AbilityTag, Level, Ability.NextDescription, true);
		}
	}

	FText FormattedText = FText::Format(
		FText::FromString(TEXT("<Body>{0} - {1}, </><Level>{2}</>")),
		FText::FromString(AbilityTag.ToString()),
		FText::FromString(TEXT("Ability has no description, Ability Level:")),
		FText::AsNumber(Level)
	);


	return FormattedText;
}

FText UPWAbilityRegistry::GetLockedDescription(int32 Level)
{
	FText FormattedText = FText::Format(
		FText::FromString(TEXT("<Body>Spell locked until level: {0}</>")),
		FText::AsNumber(Level)
	);
	return FormattedText;
}

FText UPWAbilityRegistry::FormatTextValues(const FGameplayTag& AbilityTag, int32 Level, const FText& Description, const bool Next)
{
	const FPWAbilityInfo Ability = GetAbilityByTag(AbilityTag);
	UPWGameplayAbilityBase* AbilityDefault = Cast<UPWGameplayAbilityBase>(Ability.AbilityClass.GetDefaultObject());
	if (!AbilityDefault) return Description;

	FText FormattedText = Description;

	FNumberFormattingOptions FormatOptions;
	FormatOptions.SetMaximumFractionalDigits(1);
	FormatOptions.SetMinimumFractionalDigits(0);

	if (AbilityDefault)
	{
		FDescriptionNamedArguments Args;
		FormattedText = FText::FormatNamed(
			FormattedText,
			Args._Level0, Level,
			Args._Level1, Level + 1,
			Args._CD0, FText::AsNumber(AbilityDefault->GetCooldown(Level), &FormatOptions),
			Args._CD1, FText::AsNumber(AbilityDefault->GetCooldown(Level + 1), &FormatOptions)
		);
	}

	FString GeneralFormat = FString();
	if (Next)
	{
		GeneralFormat = FString::Printf(TEXT(
			"%s\n"
			"\n"
			"<CD>Cooldown: %s > %s</>"
		),
		                                *FormattedText.ToString(), *FText::AsNumber(AbilityDefault->GetCooldown(Level), &FormatOptions).ToString(), *FText::AsNumber(AbilityDefault->GetCooldown(Level + 1), &FormatOptions).ToString()
		);
	}
	else
	{
		GeneralFormat = FString::Printf(TEXT(
			"%s\n"
			"\n"
			"<CD>Cooldown: %s</>"
		),
		                                *FormattedText.ToString(), *FText::AsNumber(AbilityDefault->GetCooldown(Level), &FormatOptions).ToString()
		);
	}


	return FText::FromString(GeneralFormat);
}
