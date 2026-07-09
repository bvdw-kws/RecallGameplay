// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/Grid/RecallGridObstacleTraits.h"

#include "Engine/World.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityView.h"
#include "Simulation/Grid/RecallGridObstacleFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"

//----------------------------------------------------------------------//
// URecallGridObstacleTrait
//----------------------------------------------------------------------//
void URecallGridObstacleTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FRecallTransformFragment>();
	
	BuildContext.AddFragment<FRecallGridObstacleFragment>();
	
	BuildContext.GetMutableObjectFragmentInitializers().Add([](UObject& Owner,
		FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
		{
			auto* GridSelectionSystem = UWorld::GetSubsystem<URecallGridSelectionSubsystem>(Owner.GetWorld());
			check(GridSelectionSystem);
		
			const auto& TransformFragment = EntityView.GetFragmentData<FRecallTransformFragment>();
			auto& ObstacleFragment = EntityView.GetFragmentData<FRecallGridObstacleFragment>();

			ObstacleFragment.GridCellIndex = GridSelectionSystem->GetGridCellIndexFromPosition(TransformFragment.Position);
			GridSelectionSystem->RegisterCell(ObstacleFragment.GridCellIndex, EntityView.GetEntity());
		}
	);
}
