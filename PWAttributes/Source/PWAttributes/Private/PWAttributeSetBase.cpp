// copyright mkurtt96


#include "PWAttributeSetBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "FEffectProperties.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

UPWAttributeSetBase::UPWAttributeSetBase()
{
}

FGameplayTag UPWAttributeSetBase::GetAttributeTag(FGameplayAttribute Attribute)
{
	return *TagsToAttributes.FindKey(Attribute);
}

float UPWAttributeSetBase::GetAttributeValue(const FGameplayTag& AttributeTag) const
{
	if (const FGameplayAttribute* Attr = TagsToAttributes.Find(AttributeTag))
	{
		return GetOwningAbilitySystemComponent()->GetNumericAttribute(*Attr);
	}
	UE_LOG(LogTemp, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
	return 0.f;
}

void UPWAttributeSetBase::SetAttributeValue(const FGameplayTag& AttributeTag, float NewValue)
{
	if (const FGameplayAttribute* Attr = TagsToAttributes.Find(AttributeTag))
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (ASC)
			ASC->SetNumericAttributeBase(*Attr, NewValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
	}
}

FGameplayAttribute UPWAttributeSetBase::GetAttribute(const FGameplayTag& AttributeTag) const
{
	if (const FGameplayAttribute* Attr = TagsToAttributes.Find(AttributeTag))
		return *Attr;

	UE_LOG(LogTemp, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
	return FGameplayAttribute();
}

void UPWAttributeSetBase::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Data.EffectSpec.GetAllAssetTags(Props.EffectAssetTags);
	Props.Source.ASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	SetEffectObject(Props.Source, Props.Source.ASC ? Props.Source.ASC->AbilityActorInfo.Get() : nullptr);
	SetEffectObject(Props.Target, Data.Target.AbilityActorInfo.Get());

	Props.Target.ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.Target.AvatarActor);
}

void UPWAttributeSetBase::SetEffectObject(FEffectObject& Props, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		Props.AvatarActor = ActorInfo->AvatarActor.Get();
		Props.Controller = ActorInfo->PlayerController.Get();
		if (!Props.Controller && Props.AvatarActor)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.AvatarActor))
			{
				Props.Controller = Pawn->GetController();
			}
		}
		if (Props.Controller)
		{
			Props.Character = Cast<ACharacter>(Props.Controller->GetPawn());
			if (Props.Controller->PlayerState)
				Props.PlayerNetId = Props.Controller->PlayerState->GetUniqueId();
		}
		Props.bIsValid = true;
	}
}
