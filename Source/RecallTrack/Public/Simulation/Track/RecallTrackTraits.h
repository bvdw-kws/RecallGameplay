// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"
#include "Physics/RecallPhysicsTypes.h"
#include "Physics/Common/JPRPhysicsCommonShapeTypes.h"
#include "Simulation/Physics/RecallPhysicsColliderTraits.h"

#include "RecallTrackTraits.generated.h"

/**
* Trait for a track entity.
*/
UCLASS(meta=(DisplayName="RE Race Track"))
class RECALLTRACK_API URecallTrackTrait : public URecallPhysicsBodyTrait
{
	GENERATED_BODY()
	
public:
	virtual void PostLoad() override;
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	UPROPERTY(EditAnywhere, Category=Collision, meta=(ShowOnlyInnerProperties))
	FJPRPhysicsMeshShapeSettings MeshShapeSettings;

	UPROPERTY()
	TObjectPtr<class ARecallTrackEntityActor> TrackActor;
};
