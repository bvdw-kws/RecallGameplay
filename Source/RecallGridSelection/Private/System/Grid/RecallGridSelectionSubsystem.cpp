// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "System/Grid/RecallGridSelectionSubsystem.h"

#include "Actor/RecallGridActor.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

void URecallGridSelectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URecallGridSelectionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URecallGridSelectionSubsystem::Reset()
{
	GridRegistry.ResetGrid();
}

void URecallGridSelectionSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallGridSelectionSubsystem::Save"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelection_Save);

	OutSnapshot.InitializeAs<FRecallGridRegistry>(GridRegistry);
}

void URecallGridSelectionSubsystem::Restore(const FRecallSnapshotContext& Context,
	const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallGridSelectionSubsystem::Restore"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelection_Restore);

	if (const FRecallGridRegistry* DataPtr = InSnapshot.GetPtr<FRecallGridRegistry>())
	{
		GridRegistry = *DataPtr;
	}
}

void URecallGridSelectionSubsystem::RegisterGridActor(ARecallGridActor* GridActor, const FName& GridName)
{
	checkf(!Recall::Simulation::Utils::HasSimulationStarted(this),
		TEXT("%hs Grid should be registered before the simulation starts"), __FUNCTION__);

	if (!ensureAlwaysMsgf(GridActor, 
		TEXT("%hs GridActor should not be nullptr"), __FUNCTION__))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!GridActorMap.Contains(GridName), 
		TEXT("%hs GridActor is already register with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}
	
	GridActorMap.Add(GridName, GridActor);

	OnGridActorRegistered.Broadcast(GridActor, GridName);
}

int32 URecallGridSelectionSubsystem::GetDefaultGridCellIndex(const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridActor->GetGridCenterCellIndex();
}

FMassEntityHandle URecallGridSelectionSubsystem::GetGridCellEntity(int32 CellIndex,
	const FName& GridName) const
{
	return GridRegistry.GetCellEntity(CellIndex);
}

int32 URecallGridSelectionSubsystem::GetGridCellIndex(const FIntVector& Coordinates, const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridActor->GetGridCellIndex(Coordinates.X, Coordinates.Y);
}

int32 URecallGridSelectionSubsystem::GetGridCellIndexFromPosition(const FVector& WorldPosition, const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return INDEX_NONE;
	}

	return GridActor->GetGridCellIndexFromWorldPosition(WorldPosition);
}

FVector URecallGridSelectionSubsystem::GetGridCellPosition(int32 CellIndex, const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return FVector::ZeroVector;
	}

	return GridActor->GetGridPosition(CellIndex);
}

int32 URecallGridSelectionSubsystem::ReserveCell(int32 CellIndex, const FName& GridName)
{
	return GridRegistry.ReserveCell(CellIndex);
}

void URecallGridSelectionSubsystem::RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity,
	const FName& GridName)
{
	GridRegistry.RegisterCell(CellIndex, Entity);
}

void URecallGridSelectionSubsystem::RegisterCell(const FVector& Position, const FMassEntityHandle& Entity, const FName& GridName)
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}

	const int32 CellIndex = GridActor->GetGridCellIndexFromWorldPosition(Position);
	if (CellIndex != INDEX_NONE)
	{
		GridRegistry.RegisterCell(CellIndex, Entity);
	}
}

void URecallGridSelectionSubsystem::UnregisterCell(int32 CellIndex, const FName& GridName)
{
	if (CellIndex < 0)
	{
		return;
	}
	
	GridRegistry.UnregisterCell(CellIndex);
}

bool URecallGridSelectionSubsystem::IsEmptyCell(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0)
	{
		return false;
	}
	
	return GridRegistry.IsEmptyCell(CellIndex);
}

bool URecallGridSelectionSubsystem::IsEntityRegistered(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0 || GridRegistry.IsEmptyCell(CellIndex))
	{
		return false;
	}
	
	return GridRegistry.GetCellEntity(CellIndex).IsSet();
}

int32 URecallGridSelectionSubsystem::GetCellReservationNumber(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex < 0)
	{
		return 0;
	}

	return GridRegistry.GetCellReservationNumber(CellIndex);
}

int32 URecallGridSelectionSubsystem::GetGridSizeX(const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0;
	}

	return GridActor->GetGridSizeX();
}

int32 URecallGridSelectionSubsystem::GetGridSizeY(const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0;
	}

	return GridActor->GetGridSizeY();
}

int32 URecallGridSelectionSubsystem::GetGridCellSize(const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return 0;
	}

	return GridActor->GetGridCellSize();
}

FVector URecallGridSelectionSubsystem::GetGridOrigin(const FName& GridName) const
{
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return FVector::ZeroVector;
	}

	return GridActor->GetActorLocation();
}

void URecallGridSelectionSubsystem::GetEmptyCellIndices(TArray<int32>& OutEmptyCellIndices, const FName& GridName) const
{
	OutEmptyCellIndices.Reset();
	
	const TObjectPtr<ARecallGridActor> GridActor = GridActorMap.FindRef(GridName);
	if (!ensureMsgf(GridActor,
		TEXT("%hs GridActor is not registered with GridName: %s"), __FUNCTION__, *GridName.ToString()))
	{
		return;
	}
	
	const int32 TotalCells = GridActor->GetTotalCellCount();
	OutEmptyCellIndices.Reserve(TotalCells);
	
	for (int32 CellIndex = 0; CellIndex < TotalCells; ++CellIndex)
	{
		if (GridRegistry.IsEmptyCell(CellIndex))
		{
			OutEmptyCellIndices.Add(CellIndex);
		}
	}
}

FIntVector URecallGridSelectionSubsystem::GetGridCoordinatesFromIndex(int32 CellIndex, const FName& GridName) const
{
	if (CellIndex == INDEX_NONE)
	{
		return FIntVector(-1, -1, 0);
	}

	const int32 GridSizeX = GetGridSizeX(GridName);
	const int32 GridSizeY = GetGridSizeY(GridName);
	
	if (GridSizeX <= 0 || GridSizeY <= 0)
	{
		return FIntVector(-1, -1, 0);
	}

	const int32 X = CellIndex % GridSizeX;
	const int32 Y = CellIndex / GridSizeX;
	
	return FIntVector(X, Y, 0);
}

int32 URecallGridSelectionSubsystem::GetGridIndexFromCoordinatesWithOffset(int32 CellIndex, int32 OffsetX, int32 OffsetY, const FName& GridName) const
{
	if (CellIndex == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	const int32 GridSizeX = GetGridSizeX(GridName);
	const int32 GridSizeY = GetGridSizeY(GridName);
	
	if (GridSizeX <= 0 || GridSizeY <= 0)
	{
		return INDEX_NONE;
	}

	// Convert cell index to coordinates
	const int32 CurrentX = CellIndex % GridSizeX;
	const int32 CurrentY = CellIndex / GridSizeX;
	
	// Apply offsets
	const int32 NewX = CurrentX + OffsetX;
	const int32 NewY = CurrentY + OffsetY;
	
	// Check bounds
	if (NewX < 0 || NewX >= GridSizeX || NewY < 0 || NewY >= GridSizeY)
	{
		return INDEX_NONE;
	}
	
	// Convert back to cell index and validate via existing method
	const FIntVector Coordinates(NewX, NewY, 0);
	return GetGridCellIndex(Coordinates, GridName);
}

int32 URecallGridSelectionSubsystem::GetNextCellTowardsTarget(int32 CurrentCellIndex, int32 TargetCellIndex, const FName& GridName) const
{
	if (CurrentCellIndex == INDEX_NONE || TargetCellIndex == INDEX_NONE)
	{
		return INDEX_NONE;
	}
	
	// If already at target, return current cell
	if (CurrentCellIndex == TargetCellIndex)
	{
		return CurrentCellIndex;
	}
	
	// Get coordinates for both cells
	const FIntVector CurrentCoords = GetGridCoordinatesFromIndex(CurrentCellIndex, GridName);
	const FIntVector TargetCoords = GetGridCoordinatesFromIndex(TargetCellIndex, GridName);
	
	if (CurrentCoords.X < 0 || CurrentCoords.Y < 0 || TargetCoords.X < 0 || TargetCoords.Y < 0)
	{
		return INDEX_NONE;
	}
	
	// Calculate direction to target (Manhattan distance approach)
	const FIntVector Direction = TargetCoords - CurrentCoords;
	
	// Determine next step - prioritize horizontal movement first, then vertical
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
	
	// Use existing offset calculation (handles bounds checking)
	return GetGridIndexFromCoordinatesWithOffset(CurrentCellIndex, OffsetX, OffsetY, GridName);
}
