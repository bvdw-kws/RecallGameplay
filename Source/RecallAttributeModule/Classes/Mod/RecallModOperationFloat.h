// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Collection/VariableCollectionTypes.h"
#include "Templates/SubclassOf.h"

#include "RecallModOperationFloat.generated.h"

struct FRecallAttributeSet;

UENUM(BlueprintType)
enum class ERecallAttributeModNumericOperation : uint8
{
	Override,
	Add,
	Multiply,
	Min,
	Max,
};

/**
 * Generic float modifier: applies an operator (Override/Add/Multiply/Min/Max) between an incoming value and a
 * constant or calculated value. Not tied to an attribute, so it can modify any float (e.g. an effect duration).
 * The attribute set is passed so that a calculation class can read attributes.
 */
UCLASS(EditInlineNew, DefaultToInstanced, Abstract, NotBlueprintable, DisplayName="Mod Operation Float Base")
class RECALLATTRIBUTEMODULE_API URecallModOperationFloatBase : public UObject
{
	GENERATED_BODY()

public:
	virtual float ApplyModOp(const FRecallAttributeSet& AttributeSet, float InValue) const;

protected:
	/** Attribute handed to the calculation context as the "current" attribute. Empty when not modifying an attribute. */
	virtual const FGameplayTag& GetContextAttribute() const { return FGameplayTag::EmptyTag; }

protected:
	UPROPERTY(EditAnywhere, Category="Operation")
	ERecallAttributeModNumericOperation Operator = ERecallAttributeModNumericOperation::Multiply;

	UPROPERTY(EditAnywhere, Category="Operation")
	FVariableCollectionFloat Value = 1.0f;

	UPROPERTY(EditAnywhere, Category="Operation")
	TSubclassOf<class URecallAttributeModMagnitudeCalculation> CalculationClass;

private:
	float GetValue(const FRecallAttributeSet& AttributeSet) const;
};

/**
 * Plain float modifier, for values that are not attributes (e.g. an effect duration).
 * Deliberately a sibling of URecallAttributeModOperationFloat rather than its base, so a property typed with this
 * class cannot be assigned an attribute operation.
 */
UCLASS(NotBlueprintable, DisplayName="Mod Operation Float")
class RECALLATTRIBUTEMODULE_API URecallModOperationFloat : public URecallModOperationFloatBase
{
	GENERATED_BODY()
};
