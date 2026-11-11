// copyright mkurtt96


#include "GAS/Abilities/PWAbilityRegistry.h"

#include "GameplayAbilitySpec.h"
#include "GAS/Abilities/AbilityDataAsset.h"
#include "GAS/Abilities/PWAbilityInfo.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"

struct FDescriptionNamedArguments
{
	FString _Level0 = FString::Printf(TEXT("_Level0"));
	FString _Level1 = FString::Printf(TEXT("_Level1"));
	FString _CD0 = FString::Printf(TEXT("_CD0"));
	FString _CD1 = FString::Printf(TEXT("_CD1"));
	FString _Cost0 = FString::Printf(TEXT("_Cost0"));
	FString _Cost1 = FString::Printf(TEXT("_Cost1"));
};

FPWAbilityInfo UPWAbilityRegistry::GetAbilityByTag(const FGameplayTag& AbilityTag)
{
	for (const auto SoftAbilityDA : Abilities)
	{
		UAbilityDataAsset* AbilityDA = SoftAbilityDA.LoadSynchronous();
		if (AbilityDA && AbilityDA->Ability.Tag.MatchesTagExact(AbilityTag))
		{
			return AbilityDA->Ability;
		}
	}
	return FPWAbilityInfo();
}

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
