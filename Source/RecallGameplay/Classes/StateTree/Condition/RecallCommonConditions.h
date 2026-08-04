// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeConditionBase.h"
#include "Data/GameplayTag/RecallGameplayTagConditionTypes.h"
#include "Mass/EntityHandle.h"
#include "Mass/EntityElementTypes.h"

#include "RecallCommonConditions.generated.h"

USTRUCT()
struct RECALLGAMEPLAY_API FRecallCompareEntityConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	FMassEntityHandle Left;

	UPROPERTY(EditAnywhere, Category=Parameter)
	FMassEntityHandle Right;
};
UE_STATETREE_ZEROED_TRIVIALLY_COPIED_NO_DESTRUCTOR_INSTANCEDATA(FRecallCompareEntityConditionInstanceData);

USTRUCT(DisplayName="Entity Compare")
struct RECALLGAMEPLAY_API FRecallCompareEntityCondition : public FRecallStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallCompareEntityConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallGameplayTagFilterConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	TArray<FMassEntityHandle> Entities;

	UPROPERTY(EditAnywhere, Category="Parameter")
	FRecallGameplayTagCondition GameplayTagCondition;
};
UE_STATETREE_CONSTRUCTED_TRIVIALLY_COPIED_NO_DESTRUCTOR_INSTANCEDATA(FRecallGameplayTagFilterConditionInstanceData);

/**
 * Evaluate if any of the entities in the array meet the gameplay tag condition.
 */
USTRUCT(DisplayName="Gameplay Tag Filter")
struct RECALLGAMEPLAY_API FRecallGameplayTagFilterCondition : public FRecallStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallGameplayTagFilterConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};
