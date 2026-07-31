// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Mass/EntityElementTypes.h"
#include "Mass/EntityHandle.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "Representation/Actor/RecallActorMeshRepresentationTypes.h"

#include "RecallGridCursorFragments.generated.h"

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridSelectionFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 GridCellIndex = INDEX_NONE;
};

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridCursorOwnerFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle GridSelectionEntity;

	UPROPERTY(VisibleAnywhere)
	FRecallActorHandle CursorActorHandle;

	UPROPERTY(VisibleAnywhere)
	bool bUseMousePosition = false;
	
	UPROPERTY(VisibleAnywhere)
	FVector2D CursorPosition = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	bool bHideCursor = false;
};

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridCursorOwnerConstSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, meta=(AllowedClasses="/Script/MassSpawner.MassEntityConfigAsset"))
	FSoftObjectPath GridSelectionEntityConfig;

	UPROPERTY(VisibleAnywhere)
	FActorRepresentationDesc CursorActorConfig;

	UPROPERTY(VisibleAnywhere)
	bool bAllowDeselectGridCell = false;
};
