// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridCursorEvaluators.h"

#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Grid/RecallGridCursorFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"

//----------------------------------------------------------------------//
// FRecallGridCursorSelectionEvaluator
//----------------------------------------------------------------------//
bool FRecallGridCursorSelectionEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(GridCursorOwnerFragmentHandle);
	Linker.LinkExternalData(GridSelectionSystemHandle);
	return true;
}

void FRecallGridCursorSelectionEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	Super::TreeStart(Context);
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bFoundCell = false;
	InstanceData.GridCellIndex = INDEX_NONE;
	InstanceData.GridCellPosition = FVector::ZeroVector;
	InstanceData.bIsGridCellEmpty = false;
	InstanceData.GridCellEntity.Reset();
}

void FRecallGridCursorSelectionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const auto& RecallContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();
	
	const FRecallGridCursorOwnerFragment& CursorOwnerFragment = Context.GetExternalData(GridCursorOwnerFragmentHandle);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (EntityManager.IsEntityValid(CursorOwnerFragment.GridSelectionEntity))
	{
		const URecallGridSelectionSubsystem& GridSelectionSubsystem = Context.GetExternalData(GridSelectionSystemHandle);
		const FMassEntityView CursorView(EntityManager, CursorOwnerFragment.GridSelectionEntity);
		const auto& CursorFragment = CursorView.GetFragmentData<FRecallGridSelectionFragment>();

		InstanceData.bFoundCell = CursorFragment.GridCellIndex != INDEX_NONE;
		InstanceData.GridCellIndex = CursorFragment.GridCellIndex;
		InstanceData.GridCellPosition = GridSelectionSubsystem.GetGridCellPosition(CursorFragment.GridCellIndex);
		InstanceData.bIsGridCellEmpty = GridSelectionSubsystem.IsEmptyCell(CursorFragment.GridCellIndex);
		InstanceData.GridCellEntity = GridSelectionSubsystem.GetGridCellEntity(CursorFragment.GridCellIndex);
	}
	else
	{
		InstanceData.bFoundCell = false;
		InstanceData.GridCellIndex = INDEX_NONE;
		InstanceData.GridCellPosition = FVector::ZeroVector;
		InstanceData.bIsGridCellEmpty = false;
		InstanceData.GridCellEntity.Reset();
	}
}
