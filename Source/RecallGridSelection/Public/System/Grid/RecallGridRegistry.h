// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"

#include "RecallGridRegistry.generated.h"

class ARecallGridActor;

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridCellEntry
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Entity;
	
	UPROPERTY(VisibleAnywhere)
	int32 CellSerialNumber = 0;
};

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridRegistry
{
	GENERATED_BODY()

public:
	int32 ReserveCell(int32 CellIndex);
	void RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity);
	void UnregisterCell(int32 CellIndex);
	bool IsEmptyCell(int32 CellIndex) const;
	FMassEntityHandle GetCellEntity(int32 CellIndex) const;
	int32 GetCellReservationNumber(int32 CellIndex) const;
	bool IsEmpty() const;
	void ResetGrid();
	
protected:
	UPROPERTY()
	TMap<int32, FRecallGridCellEntry> Cells;
	
	UPROPERTY(VisibleAnywhere)
	int32 CellSerialNumberGenerator = 0;
};

/** Immutable geometry/configuration for a grid. */
USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	int32 GridSizeX = 0;

	UPROPERTY()
	int32 GridSizeY = 0;

	UPROPERTY()
	float CellSize = 0.0f;

	UPROPERTY()
	FTransform GridTransform = FTransform::Identity;

	static FRecallGridDefinition FromGridActor(const ARecallGridActor& GridActor);
	bool IsValid() const;
	int32 GetGridCellIndex(int32 PosX, int32 PosY) const;
	int32 GetGridCellIndexFromWorldPosition(const FVector& WorldPosition) const;
	int32 GetGridCenterCellIndex() const;
	FVector GetGridPosition(int32 CellIndex) const;
};

/** Runtime grid state: geometry and rollbackable cell occupancy/reservations. */
USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridRuntimeData
{
	GENERATED_BODY()

	UPROPERTY()
	FRecallGridDefinition Definition;

	UPROPERTY()
	FRecallGridRegistry Registry;
};

/** Snapshot payload for the runtime grid map. */
USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridSelectionSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, FRecallGridRuntimeData> Grids;
};
