// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridActor.h"

#include "Engine/World.h"
#include "RecallGridCellActor.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"

ARecallGridActor::ARecallGridActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void ARecallGridActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SpawnGridCells();
}

void ARecallGridActor::BeginPlay()
{
	Super::BeginPlay();
	
	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			const int32 Index = GetGridCellIndex(X, Y);
			TObjectPtr<UChildActorComponent>& Component = ChildActorComponents[Index];
			if (!ensure(Component))
			{
				continue;
			}
			
			if (ARecallGridCellActor* ChildActor = Cast<ARecallGridCellActor>(Component->GetChildActor()))
			{
				ChildActor->SetCellCoordinates(X, Y);
				ChildActor->SetCellSize(CellSize, CellSize);
			}
		}
	}
	
	if (auto* GridSelectionSystem = UWorld::GetSubsystem<URecallGridSelectionSubsystem>(GetWorld()))
	{
		GridSelectionSystem->RegisterGridActor(this);
	}
}

void ARecallGridActor::SpawnGridCells()
{
	const int32 GridSize = GridSizeX * GridSizeY;
	const int32 OldGridSize = ChildActorComponents.Num();

	if (GridSize != OldGridSize)
	{
		for (int32 Index = GridSize; Index < OldGridSize; ++Index)
		{
			UChildActorComponent* OldComponent = ChildActorComponents[Index];
			if (IsValid(OldComponent))
			{
				OldComponent->DestroyComponent();
			}
			ChildActorComponents[Index] = nullptr;
		}
	
		ChildActorComponents.SetNum(GridSize);
	}

	const FVector CellOffset = FVector(CellSize * 0.5f, CellSize * 0.5f, 0.f);

	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			const int32 Index = GetGridCellIndex(X, Y);
			TObjectPtr<UChildActorComponent>& Component = ChildActorComponents[Index];
			if (!Component)
			{
				Component = NewObject<UChildActorComponent>(this);
				Component->SetupAttachment(RootComponent, NAME_None);
				Component->RegisterComponent();
			}
			
			Component->SetRelativeLocation(FVector(X * CellSize, Y * CellSize, 0.f) + CellOffset);
			
			if (Component->GetChildActorClass() != GridCellClass)
			{
				Component->SetChildActorClass(GridCellClass);
			}

			if (ARecallGridCellActor* ChildActor = Cast<ARecallGridCellActor>(Component->GetChildActor()))
			{
				ChildActor->SetCellSize(CellSize, CellSize);
			}
		}
	}
}

int32 ARecallGridActor::GetGridCellIndex(int32 PosX, int32 PosY) const
{
	if (PosX < 0 || PosX >= GridSizeX || PosY < 0 || PosY >= GridSizeY)
	{
		return INDEX_NONE;	
	}
	
	return GridSizeX * PosY + PosX;
}

int32 ARecallGridActor::GetGridCellIndexFromWorldPosition(const FVector& WorldPosition) const
{
	// Convert world position to local position relative to the grid actor
	const FVector LocalPosition = GetActorTransform().InverseTransformPosition(WorldPosition);
	
	// Calculate grid coordinates based on cell size
	const int32 PosX = FMath::FloorToInt(LocalPosition.X / CellSize);
	const int32 PosY = FMath::FloorToInt(LocalPosition.Y / CellSize);
	
	// Return the cell index using the existing method
	return GetGridCellIndex(PosX, PosY);
}

FVector ARecallGridActor::GetGridPosition(int32 CellIndex) const
{
	if (!ChildActorComponents.IsValidIndex(CellIndex))
	{
		return FVector::ZeroVector;
	}

	const TObjectPtr<UChildActorComponent>& Component = ChildActorComponents[CellIndex];
	if (!ensureMsgf(Component,
		TEXT("%hs Invalid cell at index: %d"), __FUNCTION__, CellIndex))
	{
		return FVector::ZeroVector;
	}
	
	return ChildActorComponents[CellIndex]->GetComponentLocation();
}

int32 ARecallGridActor::GetGridCenterCellIndex() const
{
	const int32 PosX = GridSizeX / 2;
	const int32 PosY = GridSizeY / 2;
	return GetGridCellIndex(PosX, PosY);
}
