// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class PWGASCOREEDITOR_API FPWGameplayAbility_DetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	FReply OnGenerateTagsClicked();
	static void FindOrCreateAbilityData(const class UPWGameplayAbilityBase* GA);
};
