// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "VisualAndAudio/PWMontageSet.h"

#include "Animation/AnimMontage.h"

UAnimMontage* UPWMontageSet::FindMontage(const FGameplayTag AnimTag, float& OutPlayRate) const
{
	OutPlayRate = 1.f;
	const FPWMontageEntry* Entry = Montages.Find(AnimTag);
	if (!Entry)
		return nullptr;
	OutPlayRate = Entry->PlayRate;
	return Entry->Montage;
}
