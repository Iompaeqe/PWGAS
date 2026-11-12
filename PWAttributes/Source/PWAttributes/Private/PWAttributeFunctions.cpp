// copyright mkurtt96


#include "PWAttributeFunctions.h"

#include "AbilitySystemComponent.h"
#include "PWAttributeSetBase.h"


bool UPWAttributeFunctions::ResolveAttributeOnASC(const UAbilitySystemComponent* ASC, const FGameplayTag& Tag, FGameplayAttribute& Out)
{
	if (!ASC || !Tag.IsValid()) return false;

	for (const UAttributeSet* Set : ASC->GetSpawnedAttributes())
	{
		const UPWAttributeSetBase* PWSet = Cast<UPWAttributeSetBase>(Set);
		if (const FGameplayAttribute* Attribute = PWSet ? PWSet->GetTagMap().Find(Tag) : nullptr)
		{
			Out = *Attribute;
			return true;
		}
	}
	return false;
}

FGameplayAttribute UPWAttributeFunctions::GetAttribute(const UAbilitySystemComponent* ASC, const FGameplayTag Tag)
{
	FGameplayAttribute Attr;
	ResolveAttributeOnASC(ASC, Tag, Attr);
	return Attr;
}

float UPWAttributeFunctions::GetAttributeValue(const UAbilitySystemComponent* ASC, const FGameplayTag Tag)
{
	FGameplayAttribute Attr;
	if (ResolveAttributeOnASC(ASC, Tag, Attr))
	{
		return ASC->GetNumericAttribute(Attr);
	}
	return 0.f;
}

void UPWAttributeFunctions::SetAttributeValue(UAbilitySystemComponent* ASC, const FGameplayTag Tag, const float NewBase)
{
	if (!ASC) return;

	FGameplayAttribute Attr;
	if (ResolveAttributeOnASC(ASC, Tag, Attr))
	{
		ASC->SetNumericAttributeBase(Attr, NewBase);
	}
}
