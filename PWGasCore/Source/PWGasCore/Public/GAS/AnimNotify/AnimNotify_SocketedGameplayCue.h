// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SocketedGameplayCue.generated.h"

/**
 * 
 */
UCLASS(editinlinenew, Const, hideCategories = Object, collapseCategories, Meta = (DisplayName = "GameplayCue Socket"))
class PWGASCORE_API UAnimNotify_SocketedGameplayCue : public UAnimNotify
{
	GENERATED_BODY()

public:

	UAnimNotify_SocketedGameplayCue();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif // #if WITH_EDITOR

protected:

	// GameplayCue tag to invoke.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FGameplayCueTag GameplayCue;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FName SocketName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FVector OffsetLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FRotator OffsetRotation;
};
