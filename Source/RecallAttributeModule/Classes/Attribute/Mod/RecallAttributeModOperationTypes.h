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

#include "RecallAttributeModOperationTypes.generated.h"

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

UCLASS(EditInlineNew, DefaultToInstanced, Abstract, NotBlueprintable, DisplayName="Attribute Mod Operation")
class RECALLATTRIBUTEMODULE_API URecallAttributeModOperation : public UObject
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsBaseValueMod() const { return bApplyToBaseValue; }
	FORCEINLINE const FGameplayTag& GetTargetAttribute() const { return Attribute; }
	
public:
	virtual float ApplyModOp(const FRecallAttributeSet& AttributeSet, float InValue) const;

protected:
	UPROPERTY(EditAnywhere, Category="Attribute", meta=(GameplayTagFilter="Attribute"))
	FGameplayTag Attribute = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnywhere, Category="Attribute")
	bool bApplyToBaseValue = false;
};

UCLASS(NotBlueprintable, DisplayName="Mod Operation Float")
class RECALLATTRIBUTEMODULE_API URecallAttributeModOperationFloat : public URecallAttributeModOperation
{
	GENERATED_BODY()

public:
	virtual float ApplyModOp(const FRecallAttributeSet& AttributeSet, float InValue) const override;

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
