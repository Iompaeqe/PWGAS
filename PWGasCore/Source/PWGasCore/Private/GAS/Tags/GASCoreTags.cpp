// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Tags/GASCoreTags.h"

namespace PWTags::Input // The tag "Input" is used to distinguish ability activators. Each ability should be assigned a unique tag "Input.xxxxx" and this input tag can be used to activate the ability. 
{						// See FPWASC_InputBinding.  Ex. Input.Primary, Input.SpecialAttack, Input.Jump, Input.MoveForward...
	UE_DEFINE_GAMEPLAY_TAG(Root, "Input");
	UE_DEFINE_GAMEPLAY_TAG(GlobalCancel, "Input.Cancel");
}

namespace PWTags::Event
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Event");
	UE_DEFINE_GAMEPLAY_TAG(Input, "Event.Input");
}

namespace PWTags::ASC
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "ASC");
}

namespace PWTags::MontageEvent
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "MontageEvent");
	UE_DEFINE_GAMEPLAY_TAG(Trigger, "MontageEvent.Trigger");
}

namespace PWTags::Ability
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability");
}

namespace PWTags::Ability::SetByCaller
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability.SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown, "Ability.SetByCaller.Cooldown"); // Used by dynamic Cooldown GE to set SetByCaller value.
}

namespace PWTags::Ability::Activation
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability.Activation");
	UE_DEFINE_GAMEPLAY_TAG(Failed, "Ability.Activation.Failed");
	UE_DEFINE_GAMEPLAY_TAG(OnGranted, "Ability.Activation.OnGranted");
}

namespace PWTags::Ability::Deactivation
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability.Deactivation");
	UE_DEFINE_GAMEPLAY_TAG(OnRemoved, "Ability.Deactivation.OnRemoval");
}

namespace PWTags::Ability::State
{
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability.State");
	UE_DEFINE_GAMEPLAY_TAG(AwaitingConfirmation, "Ability.State.AwaitingConfirmation");
}

namespace PWTags::Ability::Skill // The tag "Ability.Spell" is used to distinguish abilities. Each ability should be assigned a unique tag "Ability.Spell.*" and this input tag can be used to activate the ability.
								 // Each ability must have a cooldown tag as "Ability.Spell.*.Cooldown" to handle it's cooldown.
								 // IMPORTANT: These 2 tags can be generated and assigned in GA by the CallInEditor function in blueprint details
{								 // See FPWASC_InputBinding.
	UE_DEFINE_GAMEPLAY_TAG(Root, "Ability.Skill");
}

