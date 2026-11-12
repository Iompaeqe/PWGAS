// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWAttributeSetBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "FEffectProperties.h"
#include "GameplayEffectExtension.h"
#include "GameplayTagsManager.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogPWAttributes, Log, All);

UPWAttributeSetBase::UPWAttributeSetBase()
{
}

void UPWAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.Condition = COND_None;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	for (TFieldIterator<FProperty> It(GetClass()); It; ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_Net) && FGameplayAttribute::IsGameplayAttributeDataProperty(*It))
			RegisterReplicatedLifetimeProperty(*It, OutLifetimeProps, Params);
	}
}

void UPWAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampValue(Attribute, NewValue);
}

void UPWAttributeSetBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampValue(Attribute, NewValue);
}

void UPWAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (NewValue >= OldValue || !ASC || !ASC->IsOwnerActorAuthoritative())
		return;

	for (const TPair<FGameplayAttribute, FClamp>& Pair : Clamps)
	{
		if (Pair.Value.Max == Attribute && ASC->GetNumericAttributeBase(Pair.Key) > NewValue)
			ASC->SetNumericAttributeBase(Pair.Key, NewValue);
	}
}

void UPWAttributeSetBase::AddClamped(const FGameplayAttribute& Value, const FGameplayAttribute& Max, const float Min)
{
	Clamps.Add(Value, {Max, Min});
}

void UPWAttributeSetBase::ClampValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (const FClamp* Clamp = Clamps.Find(Attribute))
		NewValue = FMath::Clamp(NewValue, Clamp->Min, FMath::Max(Clamp->Min, Clamp->Max.GetNumericValue(this)));
}

const TMap<FGameplayTag, FGameplayAttribute>& UPWAttributeSetBase::GetTagMap() const
{
	if (bTagMapBuilt)
		return TagsToAttributes;

	UPWAttributeSetBase* Self = const_cast<UPWAttributeSetBase*>(this);
	Self->bTagMapBuilt = true;

	const FGameplayTag Root = AttributeTagRoot.IsValid() ? AttributeTagRoot : FGameplayTag::RequestGameplayTag(TEXT("Attribute"), false);
	if (!Root.IsValid())
		return TagsToAttributes;

	TMap<FName, FGameplayTag> TagsByName;
	for (const FGameplayTag& Tag : UGameplayTagsManager::Get().RequestGameplayTagChildren(Root))
	{
		FString Leaf = Tag.ToString();
		int32 Dot;
		if (Leaf.FindLastChar(TEXT('.'), Dot))
			Leaf.RightChopInline(Dot + 1);
		if (!TagsByName.Contains(*Leaf))
			TagsByName.Add(*Leaf, Tag);
	}

	for (TFieldIterator<FProperty> It(GetClass()); It; ++It)
	{
		if (!FGameplayAttribute::IsGameplayAttributeDataProperty(*It))
			continue;

		const FGameplayAttribute Attribute(*It);
		const FGameplayTag* Tag = TagsByName.Find(It->GetFName());
		if (Tag && !TagsToAttributes.Contains(*Tag) && !TagsToAttributes.FindKey(Attribute))
			Self->TagsToAttributes.Add(*Tag, Attribute);
	}
	return TagsToAttributes;
}

FGameplayTag UPWAttributeSetBase::GetAttributeTag(FGameplayAttribute Attribute)
{
	const FGameplayTag* Tag = GetTagMap().FindKey(Attribute);
	return Tag ? *Tag : FGameplayTag();
}

float UPWAttributeSetBase::GetAttributeValue(const FGameplayTag& AttributeTag) const
{
	if (const FGameplayAttribute* Attr = GetTagMap().Find(AttributeTag))
	{
		const UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		return ASC ? ASC->GetNumericAttribute(*Attr) : Attr->GetNumericValue(this);
	}
	UE_LOG(LogPWAttributes, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
	return 0.f;
}

void UPWAttributeSetBase::SetAttributeValue(const FGameplayTag& AttributeTag, float NewValue)
{
	if (const FGameplayAttribute* Attr = GetTagMap().Find(AttributeTag))
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (ASC)
			ASC->SetNumericAttributeBase(*Attr, NewValue);
	}
	else
	{
		UE_LOG(LogPWAttributes, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
	}
}

FGameplayAttribute UPWAttributeSetBase::GetAttribute(const FGameplayTag& AttributeTag) const
{
	if (const FGameplayAttribute* Attr = GetTagMap().Find(AttributeTag))
		return *Attr;

	UE_LOG(LogPWAttributes, Warning, TEXT("[PWAttrBase] Attribute not found for tag: %s"), *AttributeTag.ToString());
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
