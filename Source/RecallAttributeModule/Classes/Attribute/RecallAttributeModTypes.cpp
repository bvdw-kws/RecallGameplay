// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAttributeModTypes.h"

#include "RecallAttributeSetTypes.h"
#include "Mod/RecallAttributeModOperationTypes.h"

//----------------------------------------------------------------------//
// URecallAttributeMod
//----------------------------------------------------------------------//
void URecallAttributeMod::ApplyModBaseValue(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute,
	float& BaseValue) const
{
	ApplyModValue_Internal(AttributeSet, Attribute, BaseValue, true);
}

void URecallAttributeMod::ApplyModValue(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute,
	float& Value) const
{
	ApplyModValue_Internal(AttributeSet, Attribute, Value, false);
}

void URecallAttributeMod::ApplyModInstant(FRecallAttributeSet& AttributeSet) const
{
	const uint32 Handle = AttributeSet.AddMod(GetClass());

	TMap<FGameplayTag, float> NewValues;
	NewValues.Reserve(ModOperations.Num());
	
	for (const URecallAttributeModOperationFloat* Operation : ModOperations)
	{
		if (IsValid(Operation))
		{
			const FGameplayTag& Attribute = Operation->GetTargetAttribute();
			NewValues.Add(Attribute, AttributeSet.GetValue(Attribute));
		}
	}

	AttributeSet.RemoveModByHandle(Handle);
	
	for (const URecallAttributeModOperationFloat* Operation : ModOperations)
	{
		if (IsValid(Operation))
		{
			const FGameplayTag& Attribute = Operation->GetTargetAttribute();
			AttributeSet.SetValue(Attribute, NewValues[Attribute]);
		}
	}
}

void URecallAttributeMod::ApplyModValue_Internal(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute,
	float& Value, bool bIsBaseValue) const
{
	checkf(Attribute.IsValid(), TEXT("%hs Invalid attribute is not allowed"), __FUNCTION__);
	
	const TArray<URecallAttributeModOperationFloat*> ApplyModOperations = ModOperations.FilterByPredicate(
		[&Attribute, bIsBaseValue](const URecallAttributeModOperationFloat* Operation)
	{
		return IsValid(Operation)
			&& Operation->IsBaseValueMod() == bIsBaseValue
			&& Operation->GetTargetAttribute() == Attribute;	
	});
	
	for (const URecallAttributeModOperationFloat* Operation : ApplyModOperations)
	{
		Value = Operation->ApplyModOp(AttributeSet, Value);
	}
}
