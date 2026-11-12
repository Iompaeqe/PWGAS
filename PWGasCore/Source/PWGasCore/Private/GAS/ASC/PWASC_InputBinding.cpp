// copyright mkurtt96

#include "GAS/ASC/PWASC_InputBinding.h"

#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/ASC/PWASC_DataManagement.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Confirmed.h"
#include "GAS/Data/GASCoreEnums.h"
#include "GAS/Tags/GASCoreTags.h"


FPWASC_InputBinding::FPWASC_InputBinding(UPWAbilitySystemComponent& InASC) : ASC(InASC)
{
}

void FPWASC_InputBinding::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock Lock(ASC);
	if (FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfInput(InputTag))
	{
		if (!HandleAwaitingAbility(InputTag, *Spec))
		{
			ASC.AbilitySpecInputPressed(*Spec);

			if (!Spec->IsActive() && ASC.TryActivateAbility(Spec->Handle))
			{
				UE_LOG(LogGameplayTags, Log, TEXT("Activating ability: %s  through input tag: %s"), *ASC.Data().GetAbilityFromSpec(*Spec).ToString(), *InputTag.ToString());
			}
		}
	}

	ASC.SendInputEvent(InputTag, EPWInputEventType::Pressed);
}

void FPWASC_InputBinding::AbilityInputTagReleased(const FGameplayTag& InputTag) const
{
	if (!InputTag.IsValid()) return;

	FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfInput(InputTag);
	if (Spec && Spec->IsActive())
	{
		ASC.AbilitySpecInputReleased(*Spec);
	}

	ASC.SendInputEvent(InputTag, EPWInputEventType::Released);
}

void FPWASC_InputBinding::EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag) const
{
	if (!AbilityTag.IsValid() || !InputTag.IsValid()) return;

	FGameplayTag TargetAbility = ASC.Data().GetAbilityOfInput(InputTag);

	if (TargetAbility.IsValid() && TargetAbility.MatchesTagExact(AbilityTag)) return;

	SetOrSwapInputOfAbility(AbilityTag, InputTag);
}

void FPWASC_InputBinding::SetOrSwapInputOfAbility(const FGameplayTag& FirstAbilityTag, const FGameplayTag& SecondInputTag) const
{
	FGameplayAbilitySpec* FirstSpec = ASC.Data().GetSpecOfAbility(FirstAbilityTag);
	if (!FirstSpec) return;

	if (FGameplayAbilitySpec* SecondSpec = ASC.Data().GetSpecOfInput(SecondInputTag))
	{
		AssignInputToSpec(*SecondSpec, ASC.Data().GetInputFromSpec(*FirstSpec));
		ASC.MarkAbilitySpecDirty(*SecondSpec);
		AbilityInputUpdated.Broadcast(SecondInputTag);
	}

	AssignInputToSpec(*FirstSpec, SecondInputTag);
	ASC.MarkAbilitySpecDirty(*FirstSpec);
	AbilityInputUpdated.Broadcast(FirstAbilityTag);

	AbilityInputsUpdated.Broadcast();
}

void FPWASC_InputBinding::AssignInputToSpec(FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag) const
{
	ClearInputFromSpec(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
}

void FPWASC_InputBinding::ClearInputFromSpec(FGameplayAbilitySpec* Spec) const
{
	const FGameplayTag InputTag = ASC.Data().GetInputFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(InputTag);
}

bool FPWASC_InputBinding::IsInputUsed(const FGameplayTag& InputTag) const
{
	return ASC.Data().GetSpecOfInput(InputTag) ? true : false;
}

bool FPWASC_InputBinding::HandleAwaitingAbility(const FGameplayTag& InputTag, const FGameplayAbilitySpec& PressedSpec) const
{
	const FGameplayTag AbilityTag = ASC.Data().GetFirstAbilityWithTag(PWTags::Ability::State::AwaitingConfirmation);
	const FGameplayAbilitySpec* Awaiting = AbilityTag.IsValid() ? ASC.Data().GetSpecOfAbility(AbilityTag) : nullptr;
	if (!Awaiting)
		return false;

	const UPWGameplayAbility_Confirmed* Confirmed = Cast<UPWGameplayAbility_Confirmed>(Awaiting->GetPrimaryInstance());
	if (Confirmed && Confirmed->IsConfirmInput(InputTag))
		return true;

	const bool bOwnInput = Awaiting->Handle == PressedSpec.Handle;
	ASC.CancelAbilityHandle(Awaiting->Handle);
	return bOwnInput;
}
