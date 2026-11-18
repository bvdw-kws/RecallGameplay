// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallModOperationCalculation.h"

//----------------------------------------------------------------------//
// URecallModOperationCalculationContext
//----------------------------------------------------------------------//
void URecallModOperationCalculationContext::InitAttributeCalculationContext(
	const FRecallAttributeSet& InAttributeSet, const FGameplayTag& InAttribute)
{
	Attribute = InAttribute;
	AttributeSet = InAttributeSet;
}

float URecallModOperationCalculationContext::GetAttributeValue(FGameplayTag InAttribute) const
{
	if (!ensureAlwaysMsgf(Attribute != InAttribute,
		TEXT("%hs Do not call GetAttributeValue with the same attribute as the base one"), __FUNCTION__))
	{
		return 0.0f;
	}
	
	return AttributeSet.GetValue(InAttribute);
}

//----------------------------------------------------------------------//
// URecallModOperationCalculation
//----------------------------------------------------------------------//
float URecallModOperationCalculation::CalculateValue_Implementation(
	float DeltaTime, const URecallModOperationCalculationContext* Context, float BaseValue) const
{
	return BaseValue;
}
