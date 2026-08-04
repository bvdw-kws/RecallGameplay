// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridDeviceTasks.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Data/Device/RecallDeviceAsset.h"
#include "Entity/RecallGridDeviceSpawnCommand.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Grid/RecallGridObstacleFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Device/RecallDeviceSubsystem.h"
#include "System/Entity/RecallEntityAsyncSpawnSubsystem.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

//----------------------------------------------------------------------//
// FRecallGridDestroyDeviceTask
//----------------------------------------------------------------------//
bool FRecallGridDestroyDeviceTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(GridSelectionSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallGridDestroyDeviceTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.DestroyedDeviceTag = FGameplayTag::EmptyTag;

	URecallGridSelectionSubsystem& GridSelectionSystem = Context.GetExternalData(GridSelectionSystemHandle);
	if (GridSelectionSystem.IsEmptyCell(InstanceData.GridCellIndex))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (GridSelectionSystem.IsEntityRegistered(InstanceData.GridCellIndex))
	{
		const FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
		const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();

		const FMassEntityHandle CellEntity = GridSelectionSystem.GetGridCellEntity(InstanceData.GridCellIndex);
		if (EntityManager.IsEntityValid(CellEntity))
		{
			const FMassEntityView CellEntityView(EntityManager, CellEntity);
			if (auto* ObstacleFragment = CellEntityView.GetFragmentDataPtr<FRecallGridObstacleFragment>())
			{
				ObstacleFragment->GridCellIndex = INDEX_NONE;
			}

			if (const auto* GameplayTagFragment = CellEntityView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
			{
				const FGameplayTagContainer DeviceTags = Recall::GameplayTag::Utils::GetDeviceTags(GameplayTagFragment->GameplayTagCountMap);
				InstanceData.DestroyedDeviceTag = DeviceTags.First();
			}
		}
		RecallContext.GetMassExecutionContext().Defer().DestroyEntity(CellEntity);
	}

	GridSelectionSystem.UnregisterCell(InstanceData.GridCellIndex);

	return EStateTreeRunStatus::Succeeded;
}

//----------------------------------------------------------------------//
// FRecallGridSpawnDeviceTask
//----------------------------------------------------------------------//
bool FRecallGridSpawnDeviceTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(GridSelectionSystemHandle);
	Linker.LinkExternalData(EntityAsyncSpawnSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallGridSpawnDeviceTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	URecallGridSelectionSubsystem& GridSelectionSystem = Context.GetExternalData(GridSelectionSystemHandle);
	if (!GridSelectionSystem.IsEmptyCell(InstanceData.GridCellIndex))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const auto& DeviceSystem = URecallDeviceSubsystem::GetRef(Context.GetWorld());
	const TObjectPtr<const URecallDeviceAsset> DeviceAsset = DeviceSystem.GetDeviceAsset(InstanceData.Device);
	if (!DeviceAsset || DeviceAsset->EntityConfig.IsNull())
	{
		return EStateTreeRunStatus::Failed;
	}

	const FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);

	URecallEntityAsyncSpawnSubsystem& AsyncSpawnSystem = Context.GetExternalData(EntityAsyncSpawnSystemHandle);
	const FVector DevicePosition = GridSelectionSystem.GetGridCellPosition(InstanceData.GridCellIndex);

	FRecallEntityAsyncSpawnParameters SpawnParams;
	SpawnParams.EntityCount = 1;

	if (InstanceData.SpawnCommand.IsValid())
	{
		SpawnParams.SpawnCommand.InitializeAs(
			InstanceData.SpawnCommand.GetScriptStruct(), InstanceData.SpawnCommand.GetMemory());

		auto& SpawnCommand = SpawnParams.SpawnCommand.GetMutable<FRecallGridDeviceSpawnCommand>();
		SpawnCommand.OwnerEntity = RecallContext.GetEntity();
		SpawnCommand.DeviceTag = InstanceData.Device;
		SpawnCommand.GridCellIndex = InstanceData.GridCellIndex;
		SpawnCommand.GridCellReservationNumber = GridSelectionSystem.ReserveCell(InstanceData.GridCellIndex);
	}

	AsyncSpawnSystem.SpawnEntityAsync(DeviceAsset->EntityConfig, DevicePosition,
		FQuat::Identity, SpawnParams);

	return EStateTreeRunStatus::Succeeded;
}
