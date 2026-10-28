// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "System/Grid/RecallGridRegistry.h"

#include "Actor/RecallGridActor.h"

//----------------------------------------------------------------------//
// FRecallGridRegistry
//----------------------------------------------------------------------//
int32 FRecallGridRegistry::ReserveCell(int32 CellIndex)
{
	if (!ensure(IsEmptyCell(CellIndex)) && CellIndex >= 0)
	{
		return 0;
	}
	
	FRecallGridCellEntry& CellEntry = Cells.Add(CellIndex);
	CellEntry.CellSerialNumber = ++CellSerialNumberGenerator;
	return CellEntry.CellSerialNumber;
}

void FRecallGridRegistry::RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity)
{
	if (!ensure(CellIndex >= 0 && Entity.IsValid()))
	{
		return;
	}
	
	FRecallGridCellEntry& CellEntry = Cells.FindOrAdd(CellIndex);
	if (!ensureMsgf(!CellEntry.Entity.IsSet(),
		TEXT("%hs Cell is already registered"), __FUNCTION__))
	{
		return;
	}

	CellEntry.Entity = Entity;
}

void FRecallGridRegistry::UnregisterCell(int32 CellIndex)
{
	ensure(Cells.Remove(CellIndex) == 1);
}

bool FRecallGridRegistry::IsEmptyCell(int32 CellIndex) const
{
	return !Cells.Contains(CellIndex);
}

FMassEntityHandle FRecallGridRegistry::GetCellEntity(int32 CellIndex) const
{
	if (const FRecallGridCellEntry* CellPtr = Cells.Find(CellIndex))
	{
		return CellPtr->Entity;
	}
	return FMassEntityHandle();
}

int32 FRecallGridRegistry::GetCellReservationNumber(int32 CellIndex) const
{
	if (const FRecallGridCellEntry* CellPtr = Cells.Find(CellIndex))
	{
		return CellPtr->CellSerialNumber;
	}
	return 0;
}

void FRecallGridRegistry::ResetGrid()
{
	Cells.Reset();
}

//----------------------------------------------------------------------//
// FRecallGridDefinition
//----------------------------------------------------------------------//
FRecallGridDefinition FRecallGridDefinition::FromGridActor(const ARecallGridActor& GridActor)
{
	FRecallGridDefinition GridDefinition;
	GridDefinition.GridSizeX = GridActor.GetGridSizeX();
	GridDefinition.GridSizeY = GridActor.GetGridSizeY();
	GridDefinition.CellSize = GridActor.GetGridCellSize();
	GridDefinition.GridTransform = GridActor.GetActorTransform();
	return GridDefinition;
}

bool FRecallGridDefinition::IsValid() const
{
	return GridSizeX > 0 && GridSizeY > 0 && FMath::IsFinite(CellSize) && CellSize > UE_SMALL_NUMBER;
}

int32 FRecallGridDefinition::GetGridCellIndex(int32 PosX, int32 PosY) const
{
	if (PosX < 0 || PosX >= GridSizeX || PosY < 0 || PosY >= GridSizeY)
	{
		return INDEX_NONE;
	}

	return GridSizeX * PosY + PosX;
}

int32 FRecallGridDefinition::GetGridCellIndexFromWorldPosition(const FVector& WorldPosition) const
{
	if (!IsValid())
	{
		return INDEX_NONE;
	}

	const FVector LocalPosition = GridTransform.InverseTransformPosition(WorldPosition);
	const int32 PosX = FMath::FloorToInt(LocalPosition.X / CellSize);
	const int32 PosY = FMath::FloorToInt(LocalPosition.Y / CellSize);
	return GetGridCellIndex(PosX, PosY);
}

int32 FRecallGridDefinition::GetGridCenterCellIndex() const
{
	return GetGridCellIndex(GridSizeX / 2, GridSizeY / 2);
}

FVector FRecallGridDefinition::GetGridPosition(int32 CellIndex) const
{
	if (!IsValid() || CellIndex < 0 || CellIndex >= GridSizeX * GridSizeY)
	{
		return FVector::ZeroVector;
	}

	const int32 PosX = CellIndex % GridSizeX;
	const int32 PosY = CellIndex / GridSizeX;
	const FVector LocalPosition((PosX + 0.5f) * CellSize, (PosY + 0.5f) * CellSize, 0.0f);
	return GridTransform.TransformPosition(LocalPosition);
}
