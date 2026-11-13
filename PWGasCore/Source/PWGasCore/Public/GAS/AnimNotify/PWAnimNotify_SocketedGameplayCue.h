// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PWAnimNotify_SocketedGameplayCue.generated.h"

UCLASS(editinlinenew, Const, hideCategories = Object, collapseCategories, Meta = (DisplayName = "GameplayCue Socket"))
class PWGASCORE_API UPWAnimNotify_SocketedGameplayCue : public UAnimNotify
{
	GENERATED_BODY()

public:

	UPWAnimNotify_SocketedGameplayCue();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif // #if WITH_EDITOR

protected:

	// GameplayCue tag to invoke.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FGameplayCueTag GameplayCue;
	// Socket on the mesh the cue is played at.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FName SocketName;
	// Offset added to the socket location, in world space.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FVector OffsetLocation;
	// Rotation added to the socket rotation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FRotator OffsetRotation;
};
