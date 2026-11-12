#pragma once


UENUM(BlueprintType)
enum class EPWInputEventType : uint8
{
	Pressed  UMETA(DisplayName="Pressed"),
	Released UMETA(DisplayName="Released")
};
