// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGridCursorProcessors.h"

#include "Actor/Interface/RecallGridCursorActorInterface.h"
#include "Actor/Interface/RecallGridSelectionActorInterface.h"
#include "Desync/RecallDesyncLog.h"
#include "Entity/RecallGridSelectionSpawnCommand.h"
#include "Input/RecallGridSelectionInputTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Grid/RecallGridCursorFragments.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "Simulation/StateTree/RecallStateTreeProcessorGroupTypes.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Entity/RecallEntityAsyncSpawnSubsystem.h"
#include "System/Grid/RecallGridSelectionSubsystem.h"
#include "System/Input/RecallInputQueueSubsystem.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

//----------------------------------------------------------------------//
// URecallGridCursorOwnerInitializer
//----------------------------------------------------------------------//
URecallGridCursorOwnerInitializer::URecallGridCursorOwnerInitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallGridCursorOwnerFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallGridCursorOwnerInitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridCursorOwnerInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridCursorOwnerFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallGridCursorOwnerConstSharedFragment>();
	EntityQuery.AddSubsystemRequirement<URecallEntityAsyncSpawnSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallGridCursorOwnerInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& GridCursorOwnerConstSharedFragment = Context.GetConstSharedFragment<FRecallGridCursorOwnerConstSharedFragment>();
		if (GridCursorOwnerConstSharedFragment.GridSelectionEntityConfig.IsNull())
		{
			return;
		}
		
		auto& AsyncSpawnSystem = Context.GetMutableSubsystemChecked<URecallEntityAsyncSpawnSubsystem>();
		auto& ActorSystem = Context.GetMutableSubsystemChecked<URecallActorSubsystem>();
		
		const TArrayView<FRecallGridCursorOwnerFragment> CursorOwnerList = Context.GetMutableFragmentView<FRecallGridCursorOwnerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallGridCursorOwnerFragment& CursorOwnerFragment = CursorOwnerList[EntityIndex];

			// Spawn selection entity
			FRecallEntityAsyncSpawnParameters SpawnParams;
			SpawnParams.EntityCount = 1;
			SpawnParams.SpawnCommand.InitializeAs<FRecallGridSelectionSpawnCommand>();

			FRecallGridSelectionSpawnCommand& SpawnCommand = SpawnParams.SpawnCommand.GetMutable<FRecallGridSelectionSpawnCommand>();
			SpawnCommand.SelectionOwnerEntity = Context.GetEntity(EntityIndex);

			AsyncSpawnSystem.SpawnEntityAsync(GridCursorOwnerConstSharedFragment.GridSelectionEntityConfig,
				FVector::ZeroVector, FQuat::Identity, SpawnParams);

#if RECALL_DESYNC_LOG
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			RECALL_DESYNC_LOG_STR(Context.GetWorld(), "Cursor Spawn",
				FString::Printf(TEXT("Cursor spawned by: %s"), *Entity.DebugGetDescription()));
#endif // RECALL_DESYNC_LOG
			
			// Spawn cursor actor if config is provided
			if (!GridCursorOwnerConstSharedFragment.CursorActorConfig.Blueprint.IsNull())
			{
				CursorOwnerFragment.CursorActorHandle = ActorSystem.CreateActor(GridCursorOwnerConstSharedFragment.CursorActorConfig);
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridCursorOwnerDeinitializer
//----------------------------------------------------------------------//
URecallGridCursorOwnerDeinitializer::URecallGridCursorOwnerDeinitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallGridCursorOwnerFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void URecallGridCursorOwnerDeinitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridCursorOwnerDeinitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridCursorOwnerFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallGridCursorOwnerDeinitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		auto& ActorSystem = Context.GetMutableSubsystemChecked<URecallActorSubsystem>();
		const TArrayView<FRecallGridCursorOwnerFragment> CursorOwnerList = Context.GetMutableFragmentView<FRecallGridCursorOwnerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallGridCursorOwnerFragment& CursorOwnerFragment = CursorOwnerList[EntityIndex];
			
			// Clean up selection entity
			if (CursorOwnerFragment.GridSelectionEntity.IsSet())
			{
				Context.Defer().DestroyEntity(CursorOwnerFragment.GridSelectionEntity);
				CursorOwnerFragment.GridSelectionEntity.Reset();
			}

			// Clean up cursor actor
			if (CursorOwnerFragment.CursorActorHandle.IsSet())
			{
				ActorSystem.ReleaseActor(CursorOwnerFragment.CursorActorHandle);
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridSelectionInitializer
//----------------------------------------------------------------------//
URecallGridSelectionInitializer::URecallGridSelectionInitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallGridSelectionFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallGridSelectionInitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridSelectionInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridSelectionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallGridSelectionInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& GridSelectionSystem = Context.GetSubsystemChecked<URecallGridSelectionSubsystem>();
		
		const TArrayView<FRecallGridSelectionFragment> CursorList = Context.GetMutableFragmentView<FRecallGridSelectionFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallGridSelectionFragment& CursorFragment = CursorList[EntityIndex];
			CursorFragment.GridCellIndex = GridSelectionSystem.GetDefaultGridCellIndex();
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridCursorOwnerInputProcessor
//----------------------------------------------------------------------//
URecallGridCursorOwnerInputProcessor::URecallGridCursorOwnerInputProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteBefore.Add(Recall::StateTree::ProcessorGroupNames::StateTreeUpdate);
}

void URecallGridCursorOwnerInputProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridCursorOwnerInputProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridCursorOwnerFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FRecallGridCursorOwnerConstSharedFragment>();
	EntityQuery.AddSubsystemRequirement<URecallInputQueueSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallGridCursorOwnerInputProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridCursorOwnerInput_Execute);

	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Context)
	{
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
			
		const auto& GridSelectionSystem = Context.GetSubsystemChecked<URecallGridSelectionSubsystem>();
		const auto& InputQueueSystem = Context.GetSubsystemChecked<URecallInputQueueSubsystem>();

		const auto& CursorOwnerConstSharedFragment = Context.GetConstSharedFragment<FRecallGridCursorOwnerConstSharedFragment>();
	
		const TArrayView<FRecallGridCursorOwnerFragment> CursorOwnerList = Context.GetMutableFragmentView<FRecallGridCursorOwnerFragment>();
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallControllerFragment& ControllerFragment = ControllerList[EntityIndex];

			FRecallInput Input;
			if (!InputQueueSystem.GetFrameInput(ControllerFragment.ControllerID, Input))
			{
				continue;
			}

			const FRecallGridSelectionInputCommand GridSelectionInput(Input.Options);
			if (!GridSelectionInput.IsValid())
			{
				continue;
			}

			FRecallGridCursorOwnerFragment& CursorOwnerFragment = CursorOwnerList[EntityIndex];
			CursorOwnerFragment.bUseMousePosition = true;
			
			if (!EntityManager.IsEntityValid(CursorOwnerFragment.GridSelectionEntity))
			{
				continue;
			}
			
			const int32 SelectedGridCellIndex = GridSelectionSystem.GetGridCellIndex(GridSelectionInput.GridPosition);
			
			const FMassEntityView CursorView(EntityManager, CursorOwnerFragment.GridSelectionEntity);
			auto& CursorFragment = CursorView.GetFragmentData<FRecallGridSelectionFragment>();
					
			if (GridSelectionInput.IsSelect())
			{
				CursorFragment.GridCellIndex = SelectedGridCellIndex;
			}
			else if (CursorOwnerConstSharedFragment.bAllowDeselectGridCell && GridSelectionInput.IsDeselect())
			{
				if (CursorFragment.GridCellIndex == SelectedGridCellIndex)
				{
					CursorFragment.GridCellIndex = INDEX_NONE;
				}
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridSelectionPositionProcessor
//----------------------------------------------------------------------//
URecallGridSelectionPositionProcessor::URecallGridSelectionPositionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
}

void URecallGridSelectionPositionProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridSelectionPositionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallGridSelectionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallGridSelectionPositionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelectionPosition_Execute);

	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Context)
	{
		const auto& GridSelectionSystem = Context.GetSubsystemChecked<URecallGridSelectionSubsystem>();
	
		const TArrayView<FRecallTransformFragment> TransformList = Context.GetMutableFragmentView<FRecallTransformFragment>();
		const TConstArrayView<FRecallGridSelectionFragment> CursorList = Context.GetFragmentView<FRecallGridSelectionFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallGridSelectionFragment& CursorFragment = CursorList[EntityIndex];

			// Mouse the cursor to the selected cell
			FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			TransformFragment.Position = GridSelectionSystem.GetGridCellPosition(CursorFragment.GridCellIndex);
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridSelectionRepresentationProcessor
//----------------------------------------------------------------------//
URecallGridSelectionRepresentationProcessor::URecallGridSelectionRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallGridSelectionRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridSelectionRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallGridSelectionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallGridSelectionRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridSelection_Representation);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();
		const auto& GridSelectionSystem = Context.GetSubsystemChecked<URecallGridSelectionSubsystem>();
	
		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallGridSelectionFragment> CursorList = Context.GetFragmentView<FRecallGridSelectionFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

			const TWeakObjectPtr<AActor> Actor = ActorSystem.GetActor(ActorFragment.ActorHandle);
			if (!Actor.IsValid() || !Actor->GetClass()->ImplementsInterface(URecallGridSelectionActorInterface::StaticClass()))
			{
				continue;
			}

			const FRecallGridSelectionFragment& CursorFragment = CursorList[EntityIndex];
			const bool bIsEmptyCell = GridSelectionSystem.IsEmptyCell(CursorFragment.GridCellIndex);
			
			IRecallGridSelectionActorInterface::Execute_SetSelectedCellIsEmpty(Actor.Get(), bIsEmptyCell);
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridCursorPositionProcessor
//----------------------------------------------------------------------//
URecallGridCursorPositionProcessor::URecallGridCursorPositionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
}

void URecallGridCursorPositionProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridCursorPositionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridCursorOwnerFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallInputQueueSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallGridCursorPositionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridCursorPosition_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const auto& InputQueueSystem = Context.GetSubsystemChecked<URecallInputQueueSubsystem>();

		const TArrayView<FRecallGridCursorOwnerFragment> CursorOwnerList = Context.GetMutableFragmentView<FRecallGridCursorOwnerFragment>();
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallControllerFragment& ControllerFragment = ControllerList[EntityIndex];

			FRecallGridCursorOwnerFragment& CursorOwnerFragment = CursorOwnerList[EntityIndex];
			
			// Try to get mouse position from input
			FRecallInput Input;
			if (InputQueueSystem.GetFrameInput(ControllerFragment.ControllerID, Input))
			{
				CursorOwnerFragment.CursorPosition = Input.MousePosition;
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallGridCursorRepresentationProcessor
//----------------------------------------------------------------------//
URecallGridCursorRepresentationProcessor::URecallGridCursorRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallGridCursorRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGridCursorRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGridCursorOwnerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallGameplayTagFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallGridSelectionSubsystem>(EMassFragmentAccess::ReadOnly);
}

static FVector GetSelectionPosition(FMassExecutionContext& Context,
	const FMassEntityHandle& GridSelectionEntity)
{
	const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
	const auto& GridSelectionSystem = Context.GetSubsystemChecked<URecallGridSelectionSubsystem>();
	
	if (EntityManager.IsEntityValid(GridSelectionEntity))
	{
		const FMassEntityView SelectionView(EntityManager, GridSelectionEntity);
		if (const auto* SelectionFragmentPtr = SelectionView.GetFragmentDataPtr<FRecallGridSelectionFragment>())
		{
			return GridSelectionSystem.GetGridCellPosition(SelectionFragmentPtr->GridCellIndex);
		}
	}

	return FVector::ZeroVector;
}

static FVector GetGridCursorPosition(FMassExecutionContext& Context,
	const FRecallGridCursorOwnerFragment& CursorOwnerFragment)
{
	const FVector GridSelectionPosition = GetSelectionPosition(Context, CursorOwnerFragment.GridSelectionEntity);
	if (!CursorOwnerFragment.bUseMousePosition)
	{
		return GridSelectionPosition;
	}

	const APlayerController* PlayerController = Context.GetWorld()->GetFirstPlayerController();
	if (!ensure(PlayerController))
	{
		return GridSelectionPosition;
	}
	
	FVector WorldPosition = FVector::ZeroVector, WorldDirection = FVector::ZeroVector;
	const bool bDeprojected = PlayerController->DeprojectScreenPositionToWorld(
		CursorOwnerFragment.CursorPosition.X, 
		CursorOwnerFragment.CursorPosition.Y, 
		WorldPosition, 
		WorldDirection
	);
		
	if (bDeprojected)
	{
		const FPlane SelectionPlane(GridSelectionPosition, FVector::UpVector);
		const FVector LineStart = WorldPosition;
		const FVector LineEnd = WorldPosition + WorldDirection * 1000000.0f;

		float T = 0.0f;
		FVector Intersection = FVector::ZeroVector;
		if (UKismetMathLibrary::LinePlaneIntersection(LineStart, LineEnd, SelectionPlane, T, Intersection))
		{
			return Intersection;
		}
	}

	return GridSelectionPosition;
}

void URecallGridCursorRepresentationProcessor::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GridCursor_Representation);

	int32 ZOffset = 0;

	EntityQuery.ForEachEntityChunk(Context, [&ZOffset](FMassExecutionContext& Context)
	{
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		
		const auto& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();
		
		const TConstArrayView<FRecallGridCursorOwnerFragment> CursorOwnerList = Context.GetFragmentView<FRecallGridCursorOwnerFragment>();
		const TConstArrayView<FRecallGameplayTagFragment> GameplayTagList = Context.GetFragmentView<FRecallGameplayTagFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallGridCursorOwnerFragment& CursorOwnerFragment = CursorOwnerList[EntityIndex];

			const TWeakObjectPtr<AActor> CursorActor = ActorSystem.GetActor(CursorOwnerFragment.CursorActorHandle);
			if (!CursorActor.IsValid())
			{
				continue;
			}

			CursorActor->SetActorHiddenInGame(CursorOwnerFragment.bHideCursor);

			// Set owner faction on cursor actor if interface is implemented
			if (CursorActor->GetClass()->ImplementsInterface(URecallGridCursorActorInterface::StaticClass()))
			{
				FGameplayTagContainer Factions;
				if (GameplayTagList.IsValidIndex(EntityIndex))
				{
					const FRecallGameplayTagFragment& GameplayTagFragment = GameplayTagList[EntityIndex];
					Factions = Recall::GameplayTag::Utils::GetFactionTags(GameplayTagFragment.GameplayTagCountMap);
				}
				
				IRecallGridCursorActorInterface::Execute_SetOwnerFaction(CursorActor.Get(), Factions);
			}

			const FVector GridCursorPosition = GetGridCursorPosition(Context, CursorOwnerFragment);

			// Offset to avoid Z fighting between cursors.
			const FVector CursorOffset = FVector::UpVector * ZOffset++;

			CursorActor->SetActorLocation(GridCursorPosition + CursorOffset);
		}
	});
}
