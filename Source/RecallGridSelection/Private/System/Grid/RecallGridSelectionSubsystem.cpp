// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "System/Grid/RecallGridSelectionSubsystem.h"

#include "Actor/RecallGridActor.h"
#include "Async/Async.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

void URecallGridSelectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URecallGridSelectionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URecallGridSelectionSubsystem::Start(const FRecallSimulationStartParams& Params)
{
}

void URecallGridSelectionSubsystem::Reset()
{
	GridMap.Reset();
	
	for (const TPair<FName, FRecallGridDefinition>& Pair : DefaultStaticGridMap)
	{
		ARecallGridActor* GridActor = StaticGridActorMap.FindRef(Pair.Key);
		if (!ensureMsgf(IsValid(GridActor),
			TEXT("%hs Static grid actor is invalid for GridName: %s"), __FUNCTION__, *Pair.Key.ToString()))
		{
			continue;
		}

		if (AddGrid(Pair.Key, Pair.Value))
		{
			OnGridActorRegistered.Broadcast(GridActor, Pair.Key);
		}
	}
}

void URecallGridSelectionSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallGridSelectionSubsystem::Save"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelection_Save);

	FRecallGridSelectionSnapshot Snapshot;
	Snapshot.Grids = GridMap;
	OutSnapshot.InitializeAs<FRecallGridSelectionSnapshot>(Snapshot);
}

void URecallGridSelectionSubsystem::Restore(const FRecallSnapshotContext& Context,
	const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallGridSelectionSubsystem::Restore"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelection_Restore);

	if (const FRecallGridSelectionSnapshot* Snapshot = InSnapshot.GetPtr<FRecallGridSelectionSnapshot>())
	{
		GridMap = Snapshot->Grids;
	}
}

void URecallGridSelectionSubsystem::RegisterGridActor(ARecallGridActor* GridActor, const FName& GridName)
{
	checkf(!Recall::Simulation::Utils::HasSimulationStarted(this),
		TEXT("%hs Static grids should be registered before the simulation starts"), __FUNCTION__);

	if (!ensureAlwaysMsgf(GridActor,
		TEXT("%hs GridActor should not be nullptr"), __FUNCTION__))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!StaticGridActorMap.Contains(GridName),
		TEXT("%hs Static GridActor is already registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}

	const FRecallGridDefinition GridDefinition = FRecallGridDefinition::FromGridActor(*GridActor);
	if (!ensureAlwaysMsgf(GridDefinition.IsValid(),
		TEXT("%hs Static grid actor has an invalid grid definition for GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}

	StaticGridActorMap.Add(GridName, GridActor);
	DefaultStaticGridMap.Add(GridName, GridDefinition);

	if (AddGrid(GridName, GridDefinition))
	{
		OnGridActorRegistered.Broadcast(GridActor, GridName);
	}
}

bool URecallGridSelectionSubsystem::AddGrid(const FName& GridName, const FRecallGridDefinition& GridDefinition)
{
	if (!ensureMsgf(GridDefinition.IsValid(),
		TEXT("%hs Grid definition is invalid for GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return false;
	}

	if (!ensureMsgf(!GridMap.Contains(GridName),
		TEXT("%hs Grid is already registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return false;
	}

	FRecallGridRuntimeData& GridData = GridMap.Add(GridName);
	GridData.Definition = GridDefinition;
	return true;
}

bool URecallGridSelectionSubsystem::RemoveGrid(const FName& GridName)
{
	return GridMap.Remove(GridName) > 0;
}

bool URecallGridSelectionSubsystem::SetGridSize(int32 GridSizeX, int32 GridSizeY, const FName& GridName)
{
	if (!ensureMsgf(GridSizeX > 0 && GridSizeY > 0,
		TEXT("%hs Grid size must be positive"), __FUNCTION__))
	{
		return false;
	}

	FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return false;
	}

	GridData->Definition.GridSizeX = GridSizeX;
	GridData->Definition.GridSizeY = GridSizeY;
	
	TWeakObjectPtr<URecallGridSelectionSubsystem> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread, [WeakThis, GridSizeX, GridSizeY, GridName]()
	{
		if (WeakThis.IsValid())
		{
			if (ARecallGridActor* GridActor = WeakThis->StaticGridActorMap.FindRef(GridName))
			{
				GridActor->SetGridSize(GridSizeX, GridSizeY);
			}
		}
	});

	return true;
}

int32 URecallGridSelectionSubsystem::GetDefaultGridCellIndex(const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridData->Definition.GetGridCenterCellIndex();
}

int32 URecallGridSelectionSubsystem::GetGridCellIndex(const FIntVector& Coordinates, const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridData->Definition.GetGridCellIndex(Coordinates.X, Coordinates.Y);
}

int32 URecallGridSelectionSubsystem::GetGridCellIndexFromPosition(const FVector& WorldPosition,
	const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridData->Definition.GetGridCellIndexFromWorldPosition(WorldPosition);
}

bool URecallGridSelectionSubsystem::IsEntityRegistered(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0)
	{
		return false;
	}

	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	return GridData && !GridData->Registry.IsEmptyCell(CellIndex)
		&& GridData->Registry.GetCellEntity(CellIndex).IsSet();
}

FMassEntityHandle URecallGridSelectionSubsystem::GetGridCellEntity(int32 CellIndex, const FName& GridName) const
{
	if (const FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		return GridData->Registry.GetCellEntity(CellIndex);
	}

	return FMassEntityHandle();
}

FVector URecallGridSelectionSubsystem::GetGridCellPosition(int32 CellIndex, const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return FVector::ZeroVector;
	}

	return GridData->Definition.GetGridPosition(CellIndex);
}

int32 URecallGridSelectionSubsystem::ReserveCell(int32 CellIndex, const FName& GridName)
{
	if (FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		return GridData->Registry.ReserveCell(CellIndex);
	}

	return 0;
}

void URecallGridSelectionSubsystem::RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity,
	const FName& GridName)
{
	if (FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		GridData->Registry.RegisterCell(CellIndex, Entity);
	}
}

void URecallGridSelectionSubsystem::RegisterCell(const FVector& Position, const FMassEntityHandle& Entity,
	const FName& GridName)
{
	if (FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		const int32 CellIndex = GridData->Definition.GetGridCellIndexFromWorldPosition(Position);
		if (CellIndex != INDEX_NONE)
		{
			GridData->Registry.RegisterCell(CellIndex, Entity);
		}
	}
}

void URecallGridSelectionSubsystem::UnregisterCell(int32 CellIndex, const FName& GridName)
{
	if (CellIndex < 0)
	{
		return;
	}

	if (FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		GridData->Registry.UnregisterCell(CellIndex);
	}
}

bool URecallGridSelectionSubsystem::IsEmptyCell(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0)
	{
		return false;
	}

	if (const FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		return GridData->Registry.IsEmptyCell(CellIndex);
	}

	return false;
}

int32 URecallGridSelectionSubsystem::GetCellReservationNumber(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0)
	{
		return 0;
	}

	if (const FRecallGridRuntimeData* GridData = GridMap.Find(GridName))
	{
		return GridData->Registry.GetCellReservationNumber(CellIndex);
	}

	return 0;
}

int32 URecallGridSelectionSubsystem::GetGridSizeX(const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0;
	}

	return GridData->Definition.GridSizeX;
}

int32 URecallGridSelectionSubsystem::GetGridSizeY(const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0;
	}

	return GridData->Definition.GridSizeY;
}

float URecallGridSelectionSubsystem::GetGridCellSize(const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0.0f;
	}

	return GridData->Definition.CellSize;
}

FVector URecallGridSelectionSubsystem::GetGridOrigin(const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return FVector::ZeroVector;
	}

	return GridData->Definition.GridTransform.GetLocation();
}

void URecallGridSelectionSubsystem::GetEmptyCellIndices(TArray<int32>& OutEmptyCellIndices,
	const FName& GridName) const
{
	OutEmptyCellIndices.Reset();

	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (!ensureMsgf(GridData,
		TEXT("%hs Grid is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}

	const int32 TotalCells = GridData->Definition.GridSizeX * GridData->Definition.GridSizeY;
	OutEmptyCellIndices.Reserve(TotalCells);

	for (int32 CellIndex = 0; CellIndex < TotalCells; ++CellIndex)
	{
		if (GridData->Registry.IsEmptyCell(CellIndex))
		{
			OutEmptyCellIndices.Add(CellIndex);
		}
	}
}

FIntVector URecallGridSelectionSubsystem::GetGridCoordinatesFromIndex(int32 CellIndex, const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (CellIndex < 0 || !GridData || CellIndex >= GridData->Definition.GridSizeX * GridData->Definition.GridSizeY)
	{
		return FIntVector(-1, -1, 0);
	}

	const int32 X = CellIndex % GridData->Definition.GridSizeX;
	const int32 Y = CellIndex / GridData->Definition.GridSizeX;
	return FIntVector(X, Y, 0);
}

int32 URecallGridSelectionSubsystem::GetGridIndexFromCoordinatesWithOffset(int32 CellIndex, int32 OffsetX,
	int32 OffsetY, const FName& GridName) const
{
	const FRecallGridRuntimeData* GridData = GridMap.Find(GridName);
	if (CellIndex < 0 || !GridData || CellIndex >= GridData->Definition.GridSizeX * GridData->Definition.GridSizeY)
	{
		return INDEX_NONE;
	}

	const int32 CurrentX = CellIndex % GridData->Definition.GridSizeX;
	const int32 CurrentY = CellIndex / GridData->Definition.GridSizeX;
	return GridData->Definition.GetGridCellIndex(CurrentX + OffsetX, CurrentY + OffsetY);
}

int32 URecallGridSelectionSubsystem::GetNextCellTowardsTarget(int32 CurrentCellIndex, int32 TargetCellIndex,
	const FName& GridName) const
{
	if (CurrentCellIndex == INDEX_NONE || TargetCellIndex == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	if (CurrentCellIndex == TargetCellIndex)
	{
		return CurrentCellIndex;
	}

	const FIntVector CurrentCoords = GetGridCoordinatesFromIndex(CurrentCellIndex, GridName);
	const FIntVector TargetCoords = GetGridCoordinatesFromIndex(TargetCellIndex, GridName);
	if (CurrentCoords.X < 0 || CurrentCoords.Y < 0 || TargetCoords.X < 0 || TargetCoords.Y < 0)
	{
		return INDEX_NONE;
	}

	const FIntVector Direction = TargetCoords - CurrentCoords;
	int32 OffsetX = 0;
	int32 OffsetY = 0;
	if (Direction.X != 0)
	{
		OffsetX = (Direction.X > 0) ? 1 : -1;
	}
	else if (Direction.Y != 0)
	{
		OffsetY = (Direction.Y > 0) ? 1 : -1;
	}

	return GetGridIndexFromCoordinatesWithOffset(CurrentCellIndex, OffsetX, OffsetY, GridName);
}
