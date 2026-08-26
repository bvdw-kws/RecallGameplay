// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityElementTypes.h"
#include "Mass/EntityHandle.h"

#include "RecallTrackerRepresentationTypes.generated.h"

USTRUCT()
struct RECALLGAMEPLAYCORE_API FRecallTrackedEntityRepresentation
{
	GENERATED_BODY()
	
	/**
	 * Entity being tracked.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Entity;

	/**
	 * Entity location.
	 */
	UPROPERTY(VisibleAnywhere)
	FVector Location = FVector::ZeroVector;

	/**
	 * Tracker widget which will follow this entity around.
	 */
	UPROPERTY(VisibleAnywhere, meta=(MustImplement="/Script/RecallGameplayCore.RecallTrackerEntityInterface"))
	TSubclassOf<UUserWidget> TrackerWidgetClass;
};
