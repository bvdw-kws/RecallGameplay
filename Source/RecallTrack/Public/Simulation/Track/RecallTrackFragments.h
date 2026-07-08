// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Mass/EntityElementTypes.h"
#include "Physics/RecallPhysicsTypes.h"
#include "Physics/Common/JPRPhysicsCommonShapeTypes.h"

#include "RecallTrackFragments.generated.h"

USTRUCT()
struct RECALLTRACK_API FRecallTrackFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FName TrackAssetName = NAME_None;
};

USTRUCT()
struct RECALLTRACK_API FRecallTrackConstSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FJPRPhysicsBodyParameters Params;
	
	UPROPERTY(VisibleAnywhere)
	FJPRPhysicsMeshShapeSettings MeshShapeSettings;
};
