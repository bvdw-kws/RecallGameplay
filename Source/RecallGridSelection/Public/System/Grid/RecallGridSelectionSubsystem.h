// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Mass/ExternalSubsystemTraits.h"
#include "RecallGridRegistry.h"

#include "RecallGridSelectionSubsystem.generated.h"

class ARecallGridActor;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRecallGridActorRegistered, ARecallGridActor* /*GridActor*/, const FName& /*GridName*/);

UCLASS(MinimalAPI)
class URecallGridSelectionSubsystem :
	public UWorldSubsystem,
	public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	void RegisterGridActor(ARecallGridActor* GridActor, const FName& GridName = NAME_None);

	// Broadcast at the end of RegisterGridActor, i.e. from within the grid actor's own BeginPlay.
	// Since world subsystems' OnWorldBeginPlay runs before any actor's BeginPlay, code that needs
	// grid data (size, cells, etc.) as soon as it's available should bind here instead of trying
	// to read it from OnWorldBeginPlay directly.
	FOnRecallGridActorRegistered OnGridActorRegistered;

	RECALLGRIDSELECTION_API int32 GetDefaultGridCellIndex(const FName& GridName = NAME_None) const;
	
	RECALLGRIDSELECTION_API int32 GetGridCellIndex(const FIntVector& Coordinates, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API int32 GetGridCellIndexFromPosition(const FVector& WorldPosition, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API bool IsEntityRegistered(int32 CellIndex, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API FMassEntityHandle GetGridCellEntity(int32 CellIndex, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API FVector GetGridCellPosition(int32 CellIndex, const FName& GridName = NAME_None) const;
	
	RECALLGRIDSELECTION_API bool IsEmptyCell(int32 CellIndex, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API void GetEmptyCellIndices(TArray<int32>& OutEmptyCellIndices, const FName& GridName = NAME_None) const;

	RECALLGRIDSELECTION_API int32 GetGridSizeX(const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API int32 GetGridSizeY(const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API int32 GetGridCellSize(const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API FVector GetGridOrigin(const FName& GridName = NAME_None) const;
	
	// Grid coordinate conversion utilities
	RECALLGRIDSELECTION_API FIntVector GetGridCoordinatesFromIndex(int32 CellIndex, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API int32 GetGridIndexFromCoordinatesWithOffset(int32 CellIndex, int32 OffsetX, int32 OffsetY, const FName& GridName = NAME_None) const;
	
	// Cursor navigation utility
	RECALLGRIDSELECTION_API int32 GetNextCellTowardsTarget(int32 CurrentCellIndex, int32 TargetCellIndex, const FName& GridName = NAME_None) const;
	
	int32 ReserveCell(int32 CellIndex, const FName& GridName = NAME_None);
	void RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity, const FName& GridName = NAME_None);
	void RegisterCell(const FVector& Position, const FMassEntityHandle& Entity, const FName& GridName = NAME_None);
	void UnregisterCell(int32 CellIndex, const FName& GridName = NAME_None);
	
	int32 GetCellReservationNumber(int32 CellIndex, const FName& GridName = NAME_None) const;
	
protected:
	// UWorldSubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final;
	// UWorldSubsystem implementation End
	
	// IRecallSimulationReactSystemInterface implementation Begin
	virtual void Reset() override final;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override final;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override final;
	// IRecallSimulationReactSystemInterface implementation End

private:
	UPROPERTY(Transient)
	FRecallGridRegistry GridRegistry;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<ARecallGridActor>> GridActorMap;
};

template<>
struct TMassExternalSubsystemTraits<URecallGridSelectionSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
