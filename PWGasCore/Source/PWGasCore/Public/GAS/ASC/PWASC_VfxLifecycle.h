// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "GameplayTagContainer.h"


class UNiagaraComponent;
struct FPWNiagaraEntry;

class PWGASCORE_API FPWASC_VfxLifecycle
{
public:
	friend class UPWAbilitySystemComponent;
	explicit FPWASC_VfxLifecycle(UPWAbilitySystemComponent& InASC) : ASC(InASC){}
	virtual ~FPWASC_VfxLifecycle() = default;
	UPWAbilitySystemComponent& ASC;

	UNiagaraComponent* PlayNiagara_Local(const FPWNiagaraEntry& Entry);
	void StopNiagara_Local(int32 UniqueId);
	void StopNiagaraList_Local(const TArray<int32>& Ids);

protected:
	TMap<int32, TWeakObjectPtr<UNiagaraComponent>> ActiveNiagaraVfx;
};
