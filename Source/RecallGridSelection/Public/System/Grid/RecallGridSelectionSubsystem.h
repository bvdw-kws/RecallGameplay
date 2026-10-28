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
	RECALLGRIDSELECTION_API bool AddGrid(const FName& GridName, const FRecallGridDefinition& GridDefinition);
	RECALLGRIDSELECTION_API bool RemoveGrid(const FName& GridName);

	// Broadcast from Start after the static actor's grid has been copied into the runtime grid map.
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
	RECALLGRIDSELECTION_API float GetGridCellSize(const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API FVector GetGridOrigin(const FName& GridName = NAME_None) const;
	
	// Grid coordinate conversion utilities
	RECALLGRIDSELECTION_API FIntVector GetGridCoordinatesFromIndex(int32 CellIndex, const FName& GridName = NAME_None) const;
	RECALLGRIDSELECTION_API int32 GetGridIndexFromCoordinatesWithOffset(int32 CellIndex, int32 OffsetX, int32 OffsetY, const FName& GridName = NAME_None) const;
	
	// Cursor navigation utility
	RECALLGRIDSELECTION_API int32 GetNextCellTowardsTarget(int32 CurrentCellIndex, int32 TargetCellIndex, const FName& GridName = NAME_None) const;
	
	RECALLGRIDSELECTION_API int32 ReserveCell(int32 CellIndex, const FName& GridName = NAME_None);
	RECALLGRIDSELECTION_API void RegisterCell(int32 CellIndex, const FMassEntityHandle& Entity, const FName& GridName = NAME_None);
	RECALLGRIDSELECTION_API void RegisterCell(const FVector& Position, const FMassEntityHandle& Entity, const FName& GridName = NAME_None);
	RECALLGRIDSELECTION_API void UnregisterCell(int32 CellIndex, const FName& GridName = NAME_None);
	
	RECALLGRIDSELECTION_API int32 GetCellReservationNumber(int32 CellIndex, const FName& GridName = NAME_None) const;
	
public:
	// UWorldSubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final;
	// UWorldSubsystem implementation End
	
	// IRecallSimulationReactSystemInterface implementation Begin
	virtual int32 GetStartOrderPriority() const override final { return Recall::SimReactSystem::StartOrder::HighPriority; }
	virtual void Start(const FRecallSimulationStartParams& Params) override final;
	virtual void Reset() override final;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override final;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override final;
	// IRecallSimulationReactSystemInterface implementation End

private:
	UPROPERTY(Transient)
	TMap<FName, FRecallGridRuntimeData> GridMap;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<ARecallGridActor>> StaticGridActorMap;
};

template<>
struct TMassExternalSubsystemTraits<URecallGridSelectionSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
