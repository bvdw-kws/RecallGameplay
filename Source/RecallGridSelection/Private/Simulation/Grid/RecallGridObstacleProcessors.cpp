// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridObstacleProcessors.h"

#include "MassExecutionContext.h"
#include "Simulation/Grid/RecallGridObstacleFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"

//----------------------------------------------------------------------//
// URecallGridObstacleRegisterProcessor
//----------------------------------------------------------------------//
URecallGridObstacleRegisterProcessor::URecallGridObstacleRegisterProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
}

void URecallGridObstacleRegisterProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridObstacleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRecallGridObstacleRegisteredTag>(EMassFragmentPresence::None);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallGridObstacleRegisterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallGridSelectionSubsystem& GridSelectionSubsystem = Context.GetMutableSubsystemChecked<URecallGridSelectionSubsystem>();
		const TArrayView<FRecallGridObstacleFragment> ObstacleList = Context.GetMutableFragmentView<FRecallGridObstacleFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallGridObstacleFragment& ObstacleFragment = ObstacleList[EntityIndex];
			ObstacleFragment.GridCellIndex = GridSelectionSubsystem.GetGridCellIndexFromPosition(TransformList[EntityIndex].Position);

			if (ObstacleFragment.GridCellIndex == INDEX_NONE)
			{
				continue;
			}

			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			GridSelectionSubsystem.RegisterCell(ObstacleFragment.GridCellIndex, Entity);
		}
		
		Context.Defer().PushCommand<FMassCommandAddTag<FRecallGridObstacleRegisteredTag>>(
			Context.GetEntities());
	});
}

//----------------------------------------------------------------------//
// URecallGridObstacleDeinitializer
//----------------------------------------------------------------------//
URecallGridObstacleDeinitializer::URecallGridObstacleDeinitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallGridObstacleFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void URecallGridObstacleDeinitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridObstacleDeinitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridObstacleFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallGridObstacleDeinitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		auto& GridSelectionSubsystem = Context.GetMutableSubsystemChecked<URecallGridSelectionSubsystem>();
		
		const TConstArrayView<FRecallGridObstacleFragment> ObstacleList = Context.GetFragmentView<FRecallGridObstacleFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallGridObstacleFragment& ObstacleFragment = ObstacleList[EntityIndex];			
			GridSelectionSubsystem.UnregisterCell(ObstacleFragment.GridCellIndex);
		}
	});
}
