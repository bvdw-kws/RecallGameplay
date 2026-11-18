// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallModOperationFloat.h"

#include "Async/Async.h"
#include "Attribute/Mod/RecallAttributeModMagnitudeCalculation.h"
#include "Settings/RecallSimulationSettings.h"

//----------------------------------------------------------------------//
// URecallModOperationFloatBase
//----------------------------------------------------------------------//
static float ApplyModOperator(ERecallAttributeModNumericOperation Operator, float ModValue, float Value)
{
	switch (Operator)
	{
	case ERecallAttributeModNumericOperation::Override:
		return ModValue;

	case ERecallAttributeModNumericOperation::Add:
		return ModValue + Value;

	case ERecallAttributeModNumericOperation::Multiply:
		return ModValue * Value;

	case ERecallAttributeModNumericOperation::Min:
		return FMath::Min(ModValue, Value);

	case ERecallAttributeModNumericOperation::Max:
		return FMath::Max(ModValue, Value);

	default:
		unimplemented();
		return Value;
	}
}

float URecallModOperationFloatBase::ApplyModOp(const FRecallAttributeSet& AttributeSet, float InValue) const
{
	const float ModValue = GetValue(AttributeSet);
	return ApplyModOperator(Operator, ModValue, InValue);
}

float URecallModOperationFloatBase::GetValue(const FRecallAttributeSet& AttributeSet) const
{
	if (CalculationClass)
	{
		// Guard against GC interference
		FGCScopeGuard GCGuard;
		auto* Context = NewObject<URecallAttributeModMagnitudeCalculationContext>(GetOuter());
		Context->InitAttributeCalculationContext(AttributeSet, GetContextAttribute());

		// Clear the async flag
		AsyncTask(ENamedThreads::GameThread, [Context = Context]() mutable {
			Context->ClearInternalFlags(EInternalObjectFlags::Async);
		});

		const auto* CalculationCDO = CalculationClass->GetDefaultObject<URecallAttributeModMagnitudeCalculation>();
		const URecallSimulationSettings* SimulationSettings = GetDefault<URecallSimulationSettings>();
		const float DeltaTime = 1.0f / static_cast<float>(SimulationSettings->FramesPerSeconds);
		return CalculationCDO->CalculateValue(DeltaTime, Context, Value);
	}
	else
	{
		return Value;
	}
}
