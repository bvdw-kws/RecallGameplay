// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallCommonConditions.h"

#include "Algo/AnyOf.h"
#include "Async/ParallelFor.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "StateTreeExecutionContext.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

//----------------------------------------------------------------------//
// FRecallCompareEntityCondition
//----------------------------------------------------------------------//
bool FRecallCompareEntityCondition::Link(FStateTreeLinker& Linker)
{
	return true;
}

bool FRecallCompareEntityCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	return (InstanceData.Left == InstanceData.Right) != bInvert;
}

//----------------------------------------------------------------------//
// FRecallGameplayTagFilterCondition
//----------------------------------------------------------------------//
bool FRecallGameplayTagFilterCondition::Link(FStateTreeLinker& Linker)
{
	return true;
}

bool FRecallGameplayTagFilterCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();
	
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FRecallGameplayTagCondition& GameplayTagCondition = InstanceData.GameplayTagCondition;
	const TArray<FMassEntityHandle>& Entities = InstanceData.Entities;

	TArray<bool> bRequirements;
	bRequirements.SetNum(Entities.Num());

	ParallelFor(bRequirements.Num(),
		[&GameplayTagCondition, &bRequirements, &EntityManager, &Entities](int32 Index)
	{
		const FMassEntityHandle& Entity = Entities[Index];
		if (EntityManager.IsEntityValid(Entity))
		{
			const FMassEntityView EntityView(EntityManager, Entity);
			const FRecallGameplayTagFragment* GameplayTagFragmentPtr = EntityView.GetFragmentDataPtr<FRecallGameplayTagFragment>();

			bRequirements[Index] = GameplayTagFragmentPtr != nullptr ? Recall::GameplayTag::Utils::EvaluateCondition(
				GameplayTagCondition, GameplayTagFragmentPtr->GameplayTagCountMap) : false;
		}
	});

	const bool bRequirement = Algo::AnyOf(bRequirements, [](bool bRequirement)
	{
		return bRequirement;
	});
	return bRequirement != bInvert;
}
