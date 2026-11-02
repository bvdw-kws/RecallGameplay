// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameFramework/Actor.h"

#include "RecallGridActor.generated.h"

class UChildActorComponent;

/**
 * An actor used to generate a grid that can be selected by a mouse or a controller.
 * The grid can be used to pass/receive input and display the selection.
 */
UCLASS(Abstract, Blueprintable, DisplayName="RE Grid Actor")
class RECALLGRIDSELECTION_API ARecallGridActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	FVector GetGridPosition(int32 CellIndex) const;
	int32 GetGridCellIndex(int32 PosX, int32 PosY) const;
	int32 GetGridCellIndexFromWorldPosition(const FVector& WorldPosition) const;
	int32 GetGridCenterCellIndex() const;
	
	int32 GetGridSizeX() const { return GridSizeX; }
	int32 GetGridSizeY() const { return GridSizeY; }
	int32 GetTotalCellCount() const { return GridSizeX * GridSizeY; }
	float GetGridCellSize() const { return CellSize; }
	
	void SetGridSize(int32 NewGridSizeX, int32 NewGridSizeY);
	
	//~ Begin AActor Interface
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	//~ End AActor Interface

protected:
	UPROPERTY(EditAnywhere, Category=Grid)
	TSubclassOf<class ARecallGridCellActor> GridCellClass;
	
	UPROPERTY(EditAnywhere, Category=Grid, meta=(ClampMin=1))
	int32 GridSizeX = 24;
	
	UPROPERTY(EditAnywhere, Category=Grid, meta=(ClampMin=1))
	int32 GridSizeY = 16;
	
	UPROPERTY(EditAnywhere, Category=Grid, meta=(Units=Centimeters, ClampMin=0.0))
	float CellSize = 100.0f;

	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> ChildActorComponents;

	void SpawnGridCells();
	void UpdateGridCells();
};
