// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/CustomCalcs/PWExecCalc_Base.h"

float UExecCalc_Base::GetCapturedAttributeValue(const FGameplayEffectCustomExecutionParameters& Params, const FGameplayEffectAttributeCaptureDefinition& Def, const FAggregatorEvaluateParameters& EvalParams, float DefaultValue)
{
	float OutValue = DefaultValue;
	Params.AttemptCalculateCapturedAttributeMagnitude(Def, EvalParams, OutValue);
	return OutValue;
}

auto UExecCalc_Base::GetCapturedAttributeValue_Clamped(const FGameplayEffectCustomExecutionParameters& Params, const FGameplayEffectAttributeCaptureDefinition& Def, const FAggregatorEvaluateParameters& EvalParams, float MinVal, float MaxVal, float DefaultValue) -> float
{
	return FMath::Clamp(GetCapturedAttributeValue(Params, Def, EvalParams, DefaultValue), MinVal, MaxVal);
}

float UExecCalc_Base::GetCapturedAttributeValue_Max(const FGameplayEffectCustomExecutionParameters& Params, const FGameplayEffectAttributeCaptureDefinition& Def, const FAggregatorEvaluateParameters& EvalParams, float MaxVal, float DefaultValue)
{
	return FMath::Max(GetCapturedAttributeValue(Params, Def, EvalParams, DefaultValue), MaxVal);
}

float UExecCalc_Base::GetCapturedAttributeValue_Min(const FGameplayEffectCustomExecutionParameters& Params, const FGameplayEffectAttributeCaptureDefinition& Def, const FAggregatorEvaluateParameters& EvalParams, float MinVal, float DefaultValue)
{
	return FMath::Min(GetCapturedAttributeValue(Params, Def, EvalParams, DefaultValue), MinVal);
}