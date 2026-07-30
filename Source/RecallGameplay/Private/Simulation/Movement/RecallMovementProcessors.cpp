// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallMovementProcessors.h"

#include "Async/ParallelFor.h"
#include "Desync/RecallDesyncLog.h"
#include "MassExecutionContext.h"
#include "Simulation/Attribute/RecallAttributeFragments.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Movement/RecallMovementFragments.h" 
#include "Simulation/Movement/RecallMovementProcessorGroupTypes.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Physics/RecallPhysicsCharacterFragments.h"
#include "Simulation/Physics/RecallPhysicsProcessorGroupTypes.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Utility/Movement/RecallMovementUtils.h"

//----------------------------------------------------------------------//
// URecallMovementProcessor
//----------------------------------------------------------------------//
URecallMovementProcessor::URecallMovementProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Movement::ProcessorGroupNames::StartPhysics::Update;
	ExecutionOrder.ExecuteAfter.Add(Recall::Physics::ProcessorGroupNames::Initialize);
	ExecutionOrder.ExecuteBefore.Add(Recall::Physics::ProcessorGroupNames::StartSimulation);
}

void URecallMovementProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FMassTagBitSet InvalidTags;

	// Block movement while traversing a nav link.
	InvalidTags.Add(FRecallNavLinkTraversalTag::StaticStruct());
	
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallGameplayTagFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FRecallAttributeFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);	
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FJPRPhysicsCharacterFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::None>(InvalidTags);
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallMovementSharedFragment>();
}

void URecallMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Movement_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallPhysicsSubsystem& PhysicsSystem = Context.GetMutableSubsystemChecked<URecallPhysicsSubsystem>();

		const FRecallMovementSharedFragment& MovementConstSharedFragment = Context.GetConstSharedFragment<FRecallMovementSharedFragment>();

		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();
		
		const TArrayView<FRecallMovementFragment> MovementList = Context.GetMutableFragmentView<FRecallMovementFragment>();
		const TArrayView<FRecallGameplayTagFragment> GameplayTagsList = Context.GetMutableFragmentView<FRecallGameplayTagFragment>();
		const TConstArrayView<FRecallAttributeFragment> AttributeList = Context.GetFragmentView<FRecallAttributeFragment>();
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();
		const TConstArrayView<FJPRPhysicsCharacterFragment> CharacterList = Context.GetFragmentView<FJPRPhysicsCharacterFragment>();

		ParallelFor(Context.GetNumEntities(), [&](int32 EntityIndex)
		{
			const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
			const FRecallPhysicsBodyView PhysicsBody = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);
			if (!ensure(PhysicsBody.IsValid()) || !PhysicsBody.IsEnabled())
			{
				return;
			}
		
			FRecallMovementFragment& MovementFragment = MovementList[EntityIndex];
			
			FRecallGameplayTagFragment* const GameplayTagsFragmentPtr = GameplayTagsList.IsValidIndex(EntityIndex) ? &GameplayTagsList[EntityIndex] : nullptr;
			const FRecallAttributeFragment* const AttributeFragmentPtr = AttributeList.IsValidIndex(EntityIndex) ? &AttributeList[EntityIndex] : nullptr;
			const FRecallControllerFragment* const ControllerFragmentPtr = ControllerList.IsValidIndex(EntityIndex) ? &ControllerList[EntityIndex] : nullptr;
			const FJPRPhysicsCharacterFragment* const CharacterFragmentPtr = CharacterList.IsValidIndex(EntityIndex) ? &CharacterList[EntityIndex] : nullptr;

			const FRecallMovementContext MovementContext = { Context,	
				MovementFragment, MovementConstSharedFragment, PhysicsBody,
				CharacterFragmentPtr, GameplayTagsFragmentPtr, AttributeFragmentPtr, ControllerFragmentPtr };
			
			Recall::Movement::Utils::HandleMovement(MovementContext);
			Recall::Movement::Utils::HandleJump(MovementContext);
		});

#if RECALL_DESYNC_LOG
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			const FMassArchetypeHandle ArchetypeHandle = EntityManager.GetArchetypeForEntity(Entity);
			int32 AbsoluteIndex, ChunkIndex;
			EntityManager.GetArchetypeInternalIndexForEntity(Entity, ArchetypeHandle, AbsoluteIndex, ChunkIndex);
			const FRecallMovementFragment& MovementFragment = MovementList[EntityIndex];
			RECALL_DESYNC_LOG_STR(Context.GetWorld(), Movement,
				FString::Printf(TEXT("%s, MovementDirection: %s, AbsoluteIndex: %d, ChunkIndex: %d"),
				*Entity.DebugGetDescription(), *MovementFragment.MovementDirection.ToString(), AbsoluteIndex, ChunkIndex));
		}
#endif // RECALL_DESYNC_LOG
	});
}
