// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridDeviceSpawnCommand.h"

#include "Engine/World.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Grid/RecallGridObstacleFragments.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

void FRecallGridDeviceSpawnCommand::OnSpawn(FMassEntityManager& System,
                                              const TArray<FMassEntityHandle>& Entities) const
{
	check(Entities.Num() == 1);
	
	const bool bValidOwner = System.IsEntityValid(OwnerEntity);

	bool bDestroyEntity = false;

	if (!bValidOwner && bDestroyIfInvalidOwner)
	{
		bDestroyEntity = true;
	}
	
	auto* GridSelectionSystem = UWorld::GetSubsystem<URecallGridSelectionSubsystem>(System.GetWorld());
	check(GridSelectionSystem);

	const FMassEntityHandle& Entity = Entities[0];

	if (GridSelectionSystem->GetCellReservationNumber(GridCellIndex) == GridCellReservationNumber)
	{
		const FMassEntityView EntityView(System, Entity);
		auto* GameplayTagFragmentPtr = EntityView.GetFragmentDataPtr<FRecallGameplayTagFragment>();
		if (GameplayTagFragmentPtr != nullptr)
		{
			if (bAddOwnerFactionTags && bValidOwner)
			{
				const FMassEntityView OwnerView(System, OwnerEntity);
				const auto* OwnerGameplayTagFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>();
				if (OwnerGameplayTagFragmentPtr != nullptr)
				{
					const FGameplayTagContainer FactionTags = Recall::GameplayTag::Utils::GetFactionTags(
					OwnerGameplayTagFragmentPtr->GameplayTagCountMap);
					if (!FactionTags.IsEmpty())
					{
						GameplayTagFragmentPtr->GameplayTagCountMap.AddTags(FactionTags);
					}
				}
			}

			if (bAddDeviceTag)
			{
				GameplayTagFragmentPtr->GameplayTagCountMap.AddTag(DeviceTag);
			}
		}

		if (GridCellIndex != INDEX_NONE)
		{
			auto* ObstacleFragmentPtr = EntityView.GetFragmentDataPtr<FRecallGridObstacleFragment>();
			if (ensure(ObstacleFragmentPtr))
			{
				ObstacleFragmentPtr->GridCellIndex = GridCellIndex;
			}

			GridSelectionSystem->RegisterCell(GridCellIndex, Entity);
		}
	}
	else
	{
		bDestroyEntity = true;
	}
	
	if (bDestroyEntity)
	{
		System.BatchDestroyEntities(Entities);
	}	
}
