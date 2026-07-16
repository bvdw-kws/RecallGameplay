// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridCursorTasks.h"

#include "Desync/RecallDesyncLog.h"
#include "Engine/World.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "RecallSignalSubsystem.h"
#include "Simulation/Grid/RecallGridCursorFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"

//----------------------------------------------------------------------//
// FRecallGridSetCursorPositionTask
//----------------------------------------------------------------------//
bool FRecallGridSetCursorPositionTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CursorOwnerFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallGridSetCursorPositionTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Validate input grid cell index
	if (InstanceData.GridCellIndex == INDEX_NONE)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FRecallStateTreeExecutionContext& MassContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = MassContext.GetEntityManager();
		
	// Get the cursor owner fragment from external data
	FRecallGridCursorOwnerFragment& CursorOwnerFragment = Context.GetExternalData(CursorOwnerFragmentHandle);
	CursorOwnerFragment.bUseMousePosition = bUseMousePosition;
	
	// Access the cursor entity and update its position
	if (EntityManager.IsEntityValid(CursorOwnerFragment.GridSelectionEntity))
	{
		const FMassEntityView CursorEntityView(EntityManager, CursorOwnerFragment.GridSelectionEntity);
		if (FRecallGridSelectionFragment* CursorFragment = CursorEntityView.GetFragmentDataPtr<FRecallGridSelectionFragment>())
		{
			CursorFragment->GridCellIndex = InstanceData.GridCellIndex;
			return EStateTreeRunStatus::Succeeded;
		}
	}
	
	return EStateTreeRunStatus::Failed;
}

//----------------------------------------------------------------------//
// FRecallGridMoveCursorToTargetTask
//----------------------------------------------------------------------//
bool FRecallGridMoveCursorToTargetTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CursorOwnerFragmentHandle);
	Linker.LinkExternalData(GridSelectionSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallGridMoveCursorToTargetTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Validate input target cell index
	if (InstanceData.TargetGridCellIndex == INDEX_NONE)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Set current position and calculate movement interval
	InstanceData.MovementTimer = 0.0f;
	
	const FRecallStateTreeExecutionContext& MassContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	MassContext.GetSignalSystem().DelaySignalEntity(
		Recall::StateTree::Signals::TickRequired, MassContext.GetEntity(), InstanceData.GetMovementInterval());

	return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus FRecallGridMoveCursorToTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// Update movement timer
	InstanceData.MovementTimer += DeltaTime;
	
	// Check if it's time to move
	if (InstanceData.MovementTimer < InstanceData.GetMovementInterval())
	{
		return Super::Tick(Context, DeltaTime);
	}
	
	// Reset timer for next movement
	InstanceData.MovementTimer = 0.0f;
	
	const FRecallStateTreeExecutionContext& MassContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = MassContext.GetEntityManager();
	const FRecallGridCursorOwnerFragment& CursorOwnerFragment = Context.GetExternalData(CursorOwnerFragmentHandle);
	
	// Get cursor entity and current position
	if (!EntityManager.IsEntityValid(CursorOwnerFragment.GridSelectionEntity))
	{
#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_STR(Context.GetWorld(), MoveCursorToTarget_Failed, FString(TEXT("Invalid GridSelectionEntity")));
#endif // RECALL_DESYNC_LOG
		return EStateTreeRunStatus::Failed;
	}

	const FMassEntityView CursorEntityView(EntityManager, CursorOwnerFragment.GridSelectionEntity);
	FRecallGridSelectionFragment* CursorFragment = CursorEntityView.GetFragmentDataPtr<FRecallGridSelectionFragment>();
	if (!CursorFragment)
	{
#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_STR(Context.GetWorld(), MoveCursorToTarget_Failed, FString(TEXT("Missing FRecallGridSelectionFragment")));
#endif // RECALL_DESYNC_LOG
		return EStateTreeRunStatus::Failed;
	}

	// Calculate next cell using grid subsystem navigation
	const URecallGridSelectionSubsystem& GridSelectionSystem = Context.GetExternalData(GridSelectionSystemHandle);
	const int32 NextCellIndex = GridSelectionSystem.GetNextCellTowardsTarget(
		CursorFragment->GridCellIndex, InstanceData.TargetGridCellIndex);

#if RECALL_DESYNC_LOG
	RECALL_DESYNC_LOG_STR(Context.GetWorld(), MoveCursorToTarget_Tick, FString::Printf(
		TEXT("Entity: %s, CurrentCell: %d, NextCell: %d, TargetCell: %d"),
		*MassContext.GetEntity().DebugGetDescription(), CursorFragment->GridCellIndex, NextCellIndex, InstanceData.TargetGridCellIndex));
#endif // RECALL_DESYNC_LOG

	if (!ensure(NextCellIndex != INDEX_NONE))
	{
#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_STR(Context.GetWorld(), MoveCursorToTarget_Failed, FString(TEXT("Invalid NextCellIndex")));
#endif // RECALL_DESYNC_LOG
		return EStateTreeRunStatus::Failed;
	}

	// Move cursor to next cell
	CursorFragment->GridCellIndex = NextCellIndex;

	// Check if reached target
	if (NextCellIndex == InstanceData.TargetGridCellIndex)
	{
#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_STR(Context.GetWorld(), MoveCursorToTarget_Succeeded, FString::Printf(
			TEXT("Entity: %s, ReachedCell: %d"), *MassContext.GetEntity().DebugGetDescription(), NextCellIndex));
#endif // RECALL_DESYNC_LOG
		return EStateTreeRunStatus::Succeeded;
	}

	MassContext.GetSignalSystem().DelaySignalEntity(
		Recall::StateTree::Signals::TickRequired, MassContext.GetEntity(), InstanceData.GetMovementInterval());

	return Super::Tick(Context, DeltaTime);
}
