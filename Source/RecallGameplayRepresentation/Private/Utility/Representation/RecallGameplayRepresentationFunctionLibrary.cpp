// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Representation/RecallGameplayRepresentationFunctionLibrary.h"

#include "Attribute/RecallAttributeDefinitionTypes.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "MassEntityView.h"
#include "Representation/Attribute/RecallAttributeRepresentationTypes.h"
#include "Representation/Carry/RecallCarryRepresentationTypes.h"
#include "Simulation/Attribute/RecallAttributeFragments.h"
#include "Simulation/Carry/RecallCarryFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "System/Inventory/RecallInventorySubsystem.h"
#include "Utility/Entity/RecallEntityUtils.h"
#include "Utility/Player/RecallPlayerUtils.h"
#include "Utility/Simulation/RecallSimulationUtils.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

static FMassEntityView GetEntityView(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return FMassEntityView();
	}

	checkf(Recall::Simulation::Utils::IsSimulationRenderPhase(WorldContextObject),
		TEXT("%hs Entities should only be accessed during render phase"), __FUNCTION__);
	
	const FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);
	if (!EntityManager.IsEntityValid(Entity))
	{
		return FMassEntityView();
	}
	
	return FMassEntityView(EntityManager, Entity);
}

static const FRecallGameplayTagFragment* GetEntityGameplayTagFragment(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{
	const FMassEntityView EntityView = GetEntityView(WorldContextObject, Entity);
	if (!EntityView.IsValid())
	{
		return nullptr;
	}
	
	return EntityView.GetFragmentDataPtr<FRecallGameplayTagFragment>();
}

FString URecallGameplayRepresentationFunctionLibrary::GetLocalPlayerID(const UObject* WorldContextObject,
	int32 PlayerIndex)
{
	return Recall::Player::Utils::GetLocalPlayerId(WorldContextObject, PlayerIndex);
}

FString URecallGameplayRepresentationFunctionLibrary::GetControllerIdByControllerTag(const UObject* WorldContextObject,
                                                                                       const FGameplayTag& ControllerTag)
{
	return Recall::Player::Utils::GetPlayerIdByControllerTag(ControllerTag);
}

int32 URecallGameplayRepresentationFunctionLibrary::GetControllerGameplayTagCount(
	const UObject* WorldContextObject, const FGameplayTag& Tag, const FString& ControllerId)
{
	const FMassEntityHandle PlayerEntity = GetPlayerEntity(WorldContextObject, ControllerId);
	return GetGameplayTagCount(WorldContextObject, Tag, PlayerEntity);
}

FMassEntityHandle URecallGameplayRepresentationFunctionLibrary::GetPlayerEntity(
	const UObject* WorldContextObject, const FString& PlayerID)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	
	checkf(Recall::Simulation::Utils::IsSimulationRenderPhase(WorldContextObject),
		TEXT("%hs Entities should only be accessed during render phase"), __FUNCTION__);
	
	FMassEntityHandle PlayerEntity;
	Recall::Player::Utils::FindPlayerEntityInWorld(World, PlayerID, PlayerEntity);
	return PlayerEntity;
}

int32 URecallGameplayRepresentationFunctionLibrary::GetGameplayTagCount(const UObject* WorldContextObject,
                                                                          const FGameplayTag& Tag, const FMassEntityHandle& Entity)
{
	const FRecallGameplayTagFragment* GameplayTagFragment = GetEntityGameplayTagFragment(WorldContextObject, Entity);
	if (GameplayTagFragment == nullptr)
	{
		return 0;
	}

	return GameplayTagFragment->GameplayTagCountMap.GetTagCount(Tag);
}

int32 URecallGameplayRepresentationFunctionLibrary::GetFactionGameplayTagCount(const UObject* WorldContextObject,
	const FGameplayTagContainer& FactionTags, const FGameplayTag& Tag)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	const URecallInventorySubsystem* InventorySystem = UWorld::GetSubsystem<URecallInventorySubsystem>(World);
	if (!ensureAlwaysMsgf(IsValid(InventorySystem),
		TEXT("%hs Invalid inventory system"), __FUNCTION__))
	{
		return 0;
	}

	TArray<FGameplayTag> FactionGameplayTagArray;
	FactionTags.GetGameplayTagArray(FactionGameplayTagArray);

	int32 TagCount = 0;
	
	for (const FGameplayTag& FactionTag : FactionGameplayTagArray)
	{
		const FRecallGameplayTagCountMap& Inventory = InventorySystem->GetInventory(FactionTag);
		TagCount += Inventory.GetTagCount(Tag);
	}

	return TagCount;
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetGameplayTagsByEntity(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{	
	const FRecallGameplayTagFragment* GameplayTagFragment = GetEntityGameplayTagFragment(WorldContextObject, Entity);
	if (GameplayTagFragment == nullptr)
	{
		return FGameplayTagContainer();
	}

	return GameplayTagFragment->GameplayTagCountMap.GetTags();
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetGameplayTagsByControllerID(
	const UObject* WorldContextObject, const FString& ControllerId)
{
	const FMassEntityHandle PlayerEntity = GetPlayerEntity(WorldContextObject, ControllerId);
	return GetGameplayTagsByEntity(WorldContextObject, PlayerEntity);
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetSubGameplayTagsByControllerID(
	const UObject* WorldContextObject, const FGameplayTag& Tag, const FString& ControllerId)
{
	const FMassEntityHandle Entity = GetPlayerEntity(WorldContextObject, ControllerId);
	return GetSubGameplayTagsByEntity(WorldContextObject, Tag, Entity);
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetSubGameplayTagsByEntity(
	const UObject* WorldContextObject, const FGameplayTag& Tag, const FMassEntityHandle& Entity)
{
	const FGameplayTagContainer Tags = GetGameplayTagsByEntity(WorldContextObject, Entity);
	return Tags.Filter(Tag.GetSingleTagContainer());
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetFactionGameplayTags(
	const UObject* WorldContextObject, int32 PlayerIndex)
{
	const FString PlayerID = GetLocalPlayerID(WorldContextObject, PlayerIndex);
	return Recall::GameplayTag::Utils::GetFactionTags(GetGameplayTagsByControllerID(WorldContextObject, PlayerID));
}

FGameplayTagContainer URecallGameplayRepresentationFunctionLibrary::GetFactionGameplayTagsByEntity(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{
	return Recall::GameplayTag::Utils::GetFactionTags(GetGameplayTagsByEntity(WorldContextObject, Entity));
}

FMassEntityHandle URecallGameplayRepresentationFunctionLibrary::GetEntityByTags(
	const UObject* WorldContextObject, const FGameplayTagContainer& Tags, TArray<FName> NameTags)
{
	checkf(Recall::Simulation::Utils::IsSimulationRenderPhase(WorldContextObject),
		TEXT("%hs Entities should only be accessed during render phase"), __FUNCTION__);
	
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;	
	const TArray<FMassEntityHandle> Entities = Recall::Entity::Utils::GetAllEntitiesByTag(
		World, Tags, NameTags, 1);
	if (Entities.Num())
	{
		return Entities[0];
	}

	return FMassEntityHandle();
}

FRecallAttributeTrackerRepresentation URecallGameplayRepresentationFunctionLibrary::GetAttributeTrackerRepresentation(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{
	FRecallAttributeTrackerRepresentation Tracker;
	
	const FMassEntityView EntityView = GetEntityView(WorldContextObject, Entity);
	if (!EntityView.IsValid())
	{
		return Tracker;
	}

	const FRecallAttributeConstSharedFragment* AttributeConstSharedFragmentPtr = EntityView.GetConstSharedFragmentDataPtr<FRecallAttributeConstSharedFragment>();
	const FRecallAttributeFragment* AttributeFragmentPtr = EntityView.GetFragmentDataPtr<FRecallAttributeFragment>();
	if (AttributeFragmentPtr == nullptr || AttributeConstSharedFragmentPtr == nullptr)
	{
		return Tracker;
	}
	
	TArray<FGameplayTag> AttributeTags;
	AttributeFragmentPtr->AttributeSet.GetAttributeTagArray(AttributeTags);

	for (const FGameplayTag& AttributeTag : AttributeTags)
	{
		const FRecallAttributeDefinition& AttributeDef = AttributeConstSharedFragmentPtr->AttributeSet->GetAttributeChecked(AttributeTag);

		FRecallAttributeRepresentation& AttributeRepresentation = Tracker.Attributes.Add(AttributeTag);
		AttributeRepresentation.CurrentValue = AttributeFragmentPtr->AttributeSet.GetValue(AttributeTag);
		AttributeRepresentation.MaxValue = AttributeDef.MinValue.GetValue(AttributeFragmentPtr->AttributeSet);
		AttributeRepresentation.MaxValue = AttributeDef.MaxValue.GetValue(AttributeFragmentPtr->AttributeSet);
	}

	return Tracker;
}

FRecallCarryableRepresentation URecallGameplayRepresentationFunctionLibrary::GetCarryableRepresentation(
	const UObject* WorldContextObject, const FMassEntityHandle& Entity)
{
	FRecallCarryableRepresentation Representation;
	
	const FMassEntityView EntityView = GetEntityView(WorldContextObject, Entity);
	if (!EntityView.IsSet())
	{
		return Representation;
	}
	
	if (const auto* CarryableConstSharedFragmentPtr = EntityView.GetConstSharedFragmentDataPtr<FRecallCarryableConstSharedFragment>())
	{
		Representation.CarrierRange = CarryableConstSharedFragmentPtr->Settings.CarrierRange;
	}

	if (const auto* CarryableFragmentPtr = EntityView.GetFragmentDataPtr<FRecallCarryableFragment>())
	{
		Representation.CarrierCount = CarryableFragmentPtr->GetCarrierCount();
	}
	
	return Representation;
}
