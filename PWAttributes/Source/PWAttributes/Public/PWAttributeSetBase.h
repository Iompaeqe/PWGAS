// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PWAttributeSetBase.generated.h"

struct FEffectProperties;
struct FEffectObject;
struct FGameplayTag;

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Put once in the cpp of a set with replicated attributes. The base registers them all.
#define PW_REPLICATE_ATTRIBUTES(ClassName) \
void ClassName::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const \
{ Super::GetLifetimeReplicatedProps(OutLifetimeProps); }

//Attribute set doesn't work with macros, But you can use this as snippets then substitute the macro call
#define DeclareReplicatedAttribute(PropertyName, CategoryName) \
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_##PropertyName, Category = CategoryName) \
FGameplayAttributeData PropertyName; \
UFUNCTION() void OnRep_##PropertyName(const FGameplayAttributeData& OldValue) const \
{ GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, PropertyName, OldValue); } \
ATTRIBUTE_ACCESSORS(ThisClass, PropertyName)

//Attribute set doesn't work with macros, But you can use this as snippets then substitute the macro call
#define DeclareLocalAttribute(PropertyName, CategoryName) \
UPROPERTY(BlueprintReadOnly, Category = CategoryName) \
FGameplayAttributeData PropertyName; \
ATTRIBUTE_ACCESSORS(ThisClass, PropertyName)



UCLASS()
class PWATTRIBUTES_API UPWAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPWAttributeSetBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// Attributes are matched by name to the tags under this root. Entries added by hand win.
	UPROPERTY()
	FGameplayTag AttributeTagRoot;

	UPROPERTY()
	TMap<FGameplayTag, FGameplayAttribute> TagsToAttributes;

	const TMap<FGameplayTag, FGameplayAttribute>& GetTagMap() const;

	// Keeps Value between Min and the current value of Max.
	void AddClamped(const FGameplayAttribute& Value, const FGameplayAttribute& Max, float Min = 0.f);

	UFUNCTION(BlueprintPure, Category="PW|Attributes")
	virtual FGameplayTag GetAttributeTag(FGameplayAttribute Attribute);

	UFUNCTION(BlueprintCallable, Category="PW|Attributes")
	virtual float GetAttributeValue(const FGameplayTag& AttributeTag) const;
	UFUNCTION(BlueprintCallable, Category="PW|Attributes")
	virtual void SetAttributeValue(const FGameplayTag& AttributeTag, float NewValue);
	UFUNCTION(BlueprintCallable, Category="PW|Attributes")
	virtual FGameplayAttribute GetAttribute(const FGameplayTag& AttributeTag) const;

	virtual void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props);
	virtual void SetEffectObject(FEffectObject& Props, const FGameplayAbilityActorInfo* ActorInfo);

protected:
	struct FClamp
	{
		FGameplayAttribute Max;
		float Min = 0.f;
	};
	TMap<FGameplayAttribute, FClamp> Clamps;

	void ClampValue(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	bool bTagMapBuilt = false;
};
