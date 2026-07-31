// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridCellActor.h"

#include "Components/BoxComponent.h"
#include "Input/RecallGridSelectionInputTypes.h"
#include "Utility/Grid/RecallGridSelectionFunctionLibrary.h"

ARecallGridCellActor::ARecallGridCellActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
	
	BoxComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("BoxComponent"));
	if (BoxComponent)
	{
		BoxComponent->SetupAttachment(RootComponent);
		BoxComponent->SetMobility(EComponentMobility::Movable);
	}
}

void ARecallGridCellActor::BeginPlay()
{
	Super::BeginPlay();
	
	OnBeginCursorOver.AddDynamic(this, &ThisClass::OnBeginCursorOverEvent);
	OnEndCursorOver.AddDynamic(this, &ThisClass::OnEndCursorOverEvent);
}

void ARecallGridCellActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	OnBeginCursorOver.RemoveDynamic(this, &ThisClass::OnBeginCursorOverEvent);
	OnEndCursorOver.RemoveDynamic(this, &ThisClass::OnEndCursorOverEvent);
}

void ARecallGridCellActor::SetCellSize_Implementation(float SizeX, float SizeY)
{
	if (BoxComponent)
	{
		BoxComponent->SetBoxExtent(FVector(SizeX * 0.5f, SizeY * 0.5f, CellHalfHeight));
	}
}

void ARecallGridCellActor::SetCellCoordinates_Implementation(int32 PosX, int32 PosY)
{
	CellCoordinates = FIntVector(PosX, PosY, 0);
}

void ARecallGridCellActor::OnBeginCursorOverEvent(AActor* TouchedActor)
{
	FRecallGridSelectionInputCommand GridSelectionInputCommand;
	GridSelectionInputCommand.Type = ERecallGridSelectionInputType::Select;
	GridSelectionInputCommand.GridPosition = CellCoordinates;
	
	URecallGridSelectionFunctionLibrary::SetGridSelectionInputCommand(
		this, GridSelectionInputCommand, 0);
}

void ARecallGridCellActor::OnEndCursorOverEvent(AActor* TouchedActor)
{
	FRecallGridSelectionInputCommand GridSelectionInputCommand;
	GridSelectionInputCommand.Type = ERecallGridSelectionInputType::Deselect;
	GridSelectionInputCommand.GridPosition = CellCoordinates;
	
	URecallGridSelectionFunctionLibrary::SetGridSelectionInputCommand(
		this, GridSelectionInputCommand, 0);
}
