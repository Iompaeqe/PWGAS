// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PWExecCalc_Base.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PWGASCORE_API UExecCalc_Base : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	/**
	 * Attempts to calculate captured attribute magnitude.
	 * @param Params 
	 * @param Def 
	 * @param EvalParams 
	 * @param DefaultValue 
	 * @return 
	 */
	static float GetCapturedAttributeValue(
		const FGameplayEffectCustomExecutionParameters& Params,
		const FGameplayEffectAttributeCaptureDefinition& Def,
		const FAggregatorEvaluateParameters& EvalParams,
		float DefaultValue = 0.f);
	
	/**
	 * Returns captured value clamped between Min and Max.
	 * @param Params 
	 * @param Def 
	 * @param EvalParams 
	 * @param MinVal 
	 * @param MaxVal 
	 * @param DefaultValue 
	 * @return 
	 */
	static float GetCapturedAttributeValue_Clamped(
		const FGameplayEffectCustomExecutionParameters& Params,
		const FGameplayEffectAttributeCaptureDefinition& Def,
		const FAggregatorEvaluateParameters& EvalParams,
		float MinVal, float MaxVal,
		float DefaultValue = 0.f);

	/**
	 * Returns captured value, not exceeding MaxVal.
	 * @param Params 
	 * @param Def 
	 * @param EvalParams 
	 * @param MaxVal 
	 * @param DefaultValue 
	 * @return 
	 */
	static float GetCapturedAttributeValue_Max(
		const FGameplayEffectCustomExecutionParameters& Params,
		const FGameplayEffectAttributeCaptureDefinition& Def,
		const FAggregatorEvaluateParameters& EvalParams,
		float MaxVal,
		float DefaultValue = 0.f);

	/**
	 * Returns captured value, not below MinVal.
	 * @param Params 
	 * @param Def 
	 * @param EvalParams 
	 * @param MinVal 
	 * @param DefaultValue 
	 * @return 
	 */
	static float GetCapturedAttributeValue_Min(
		const FGameplayEffectCustomExecutionParameters& Params,
		const FGameplayEffectAttributeCaptureDefinition& Def,
		const FAggregatorEvaluateParameters& EvalParams,
		float MinVal,
		float DefaultValue = 0.f);
};


