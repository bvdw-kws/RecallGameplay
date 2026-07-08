// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallActorRepresentationProcessors.h"

#include "Actor/RecallRepresentationActor.h"
#include "Animation/RecallAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "MassExecutionContext.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/Movement/RecallMovementFragments.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Physics/RecallPhysicsCharacterFragments.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "System/Representation/RecallActorInterpolationSubsystem.h"
#include "Utility/Math/RecallMathUtils.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

//----------------------------------------------------------------------//
// URecallActorRepresentationInitializer
//----------------------------------------------------------------------//
URecallActorRepresentationInitializer::URecallActorRepresentationInitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallActorRepresentationFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallActorRepresentationInitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallActorRepresentationInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallSkeletalMeshActorRepresentationConstSharedFragment>(EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FRecallStaticMeshActorRepresentationConstSharedFragment>(EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FRecallActorRepresentationConstSharedFragment>(EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FRecallDecalActorRepresentationConstSharedFragment>(EMassFragmentPresence::Optional);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallActorRepresentationInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallActorSubsystem& ActorSystem = Context.GetMutableSubsystemChecked<URecallActorSubsystem>();

		const FRecallSkeletalMeshActorRepresentationConstSharedFragment* SkeletalMeshSharedFragment = Context.GetConstSharedFragmentPtr<FRecallSkeletalMeshActorRepresentationConstSharedFragment>();
		const FRecallStaticMeshActorRepresentationConstSharedFragment* StaticMeshSharedFragment = Context.GetConstSharedFragmentPtr<FRecallStaticMeshActorRepresentationConstSharedFragment>();
		const FRecallActorRepresentationConstSharedFragment* ActorSharedFragment = Context.GetConstSharedFragmentPtr<FRecallActorRepresentationConstSharedFragment>();
		const auto* DecalActorConstSharedFragment = Context.GetConstSharedFragmentPtr<FRecallDecalActorRepresentationConstSharedFragment>();

		const TArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetMutableFragmentView<FRecallActorRepresentationFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];
			if (ActorFragment.ActorHandle.IsSet())
			{
				continue;
			}

			if (SkeletalMeshSharedFragment != nullptr)
			{
				ActorFragment.ActorHandle = ActorSystem.CreateActor(SkeletalMeshSharedFragment->Definition);
			}
			else if (StaticMeshSharedFragment != nullptr)
			{
				ActorFragment.ActorHandle = ActorSystem.CreateActor(StaticMeshSharedFragment->Definition);
			}
			else if (ActorSharedFragment != nullptr)
			{
				ActorFragment.ActorHandle = ActorSystem.CreateActor(ActorSharedFragment->Definition);
			}
			else if (DecalActorConstSharedFragment != nullptr)
			{
				ActorFragment.ActorHandle = ActorSystem.CreateActor(DecalActorConstSharedFragment->Desc);
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallActorRepresentationDeinitializer
//----------------------------------------------------------------------//
URecallActorRepresentationDeinitializer::URecallActorRepresentationDeinitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallActorRepresentationFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void URecallActorRepresentationDeinitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallActorRepresentationDeinitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadWrite);	
	EntityQuery.AddSubsystemRequirement<URecallActorInterpolationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallActorRepresentationDeinitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallActorSubsystem& ActorSystem = Context.GetMutableSubsystemChecked<URecallActorSubsystem>();
		URecallActorInterpolationSubsystem& ActorInterpolationSystem = Context.GetMutableSubsystemChecked<URecallActorInterpolationSubsystem>();

		const TArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetMutableFragmentView<FRecallActorRepresentationFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

			ActorInterpolationSystem.ClearInterpolationCache(ActorFragment.ActorHandle);
			ActorSystem.ReleaseActor(ActorFragment.ActorHandle);
		}
	});
}

static const FName RecallActorRepresentation = TEXT("RecallActorRepresentation");

//----------------------------------------------------------------------//
// URecallActorRepresentationProcessor
//----------------------------------------------------------------------//
URecallActorRepresentationProcessor::URecallActorRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	ExecutionOrder.ExecuteInGroup = RecallActorRepresentation;
	bRequiresGameThreadExecution = true;
}

void URecallActorRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallActorRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);

	// This system is used for representation only so it is safe to write.
	ProcessorRequirements.AddSubsystemRequirement<URecallActorInterpolationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallActorRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Actor_Representation);

	URecallActorInterpolationSubsystem& ActorInterpolationSystem = Context.GetMutableSubsystemChecked<URecallActorInterpolationSubsystem>();

	ActorInterpolationSystem.UpdateDeltaFrame(Context.GetDeltaTimeSeconds());
	
	EntityQuery.ForEachEntityChunk(Context,
		[&ActorInterpolationSystem](FMassExecutionContext& Context)
	{
		const URecallActorSubsystem& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();

		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];

			const TWeakObjectPtr<AActor> Actor = ActorSystem.GetActor(ActorFragment.ActorHandle);
			if (!Actor.IsValid())
			{
				continue;
			}
			
			Actor->SetActorHiddenInGame(ActorFragment.bHideActor);
			if (ActorFragment.bHideActor)
			{
				continue;
			}
			
			const FTransform RootTransform(
				TransformFragment.Rotation, TransformFragment.Position, ActorFragment.Scale);
			const FTransform Transform = ActorFragment.Offset * RootTransform;
			
			FVector NewPosition = Transform.GetLocation();
			FQuat NewRotation = Transform.GetRotation();
			
			ActorInterpolationSystem.InterpolateVector(ActorFragment.ActorHandle, TEXT("Position"), NewPosition);
			ActorInterpolationSystem.InterpolateQuat(ActorFragment.ActorHandle, TEXT("Rotation"), NewRotation);

			Actor->SetActorLocationAndRotation(NewPosition, NewRotation);

			if (Actor->GetActorScale3D() != Transform.GetScale3D())
			{
				Actor->SetActorScale3D(Transform.GetScale3D());
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallControllerActorRepresentationProcessor
//----------------------------------------------------------------------//
URecallControllerActorRepresentationProcessor::URecallControllerActorRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	ExecutionOrder.ExecuteAfter.Add(RecallActorRepresentation);
	bRequiresGameThreadExecution = true;
}

void URecallControllerActorRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallControllerActorRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// This system is used for representation only so it is safe to write.
	EntityQuery.AddSubsystemRequirement<URecallActorInterpolationSubsystem>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallControllerActorRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ControllerActor_Representation);

	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Context)
	{
		URecallActorInterpolationSubsystem& ActorInterpolationSystem = Context.GetMutableSubsystemChecked<URecallActorInterpolationSubsystem>();
			
		const URecallActorSubsystem& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();

		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

			const TWeakObjectPtr<ARecallRepresentationActor> Actor = ActorSystem.GetActor<ARecallRepresentationActor>(ActorFragment.ActorHandle);
			if (!Actor.IsValid())
			{
				continue;
			}
			
			const FRecallControllerFragment& ControllerFragment = ControllerList[EntityIndex];

			FQuat ControlRotation = ControllerFragment.ControlRotation.Quaternion();

			ActorInterpolationSystem.InterpolateQuat(ActorFragment.ActorHandle, TEXT("ControlRotation"), ControlRotation);
			
			Actor->SetControlRotationRepresentation(ControlRotation);
		}
	});
}

//----------------------------------------------------------------------//
// URecallActorAnimationRepresentationProcessor
//----------------------------------------------------------------------//
URecallActorAnimationRepresentationProcessor::URecallActorAnimationRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallActorAnimationRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallActorAnimationRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FMassTagBitSet RequiredTags;
	RequiredTags.Add(FRecallSkeletalMeshActorRepresentationTag::StaticStruct());
	
	FMassTagBitSet InvalidTags;
	// InvalidTags.Add(FRecallOverrideActorAnimationRepresentationTag::StaticStruct());
	
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallMovementFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FJPRPhysicsCharacterFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::None>(InvalidTags);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::All>(RequiredTags);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallActorAnimationRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Actor_AnimationRepresentation);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Recall::Simulation::Utils::GetDilatedFixedDeltaTime(Context.GetWorld());
		const float DeltaFrame = Recall::Simulation::Utils::GetRepresentationDeltaFrame(Context.GetWorld());

		const float AnimationDeltaTime = DeltaTime * DeltaFrame;

		const URecallActorSubsystem& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();
		const URecallPhysicsSubsystem& PhysicsSystem = Context.GetSubsystemChecked<URecallPhysicsSubsystem>();

		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallMovementFragment> MovementList = Context.GetFragmentView<FRecallMovementFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();
		const TConstArrayView<FJPRPhysicsCharacterFragment> CharacterList = Context.GetFragmentView<FJPRPhysicsCharacterFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

			const TWeakObjectPtr<AActor> Actor = ActorSystem.GetActor(ActorFragment.ActorHandle);
			if (!Actor.IsValid())
			{
				continue;
			}

			Actor->ForEachComponent<USkeletalMeshComponent>(false, [&](USkeletalMeshComponent* Component)
			{
				URecallAnimInstance* AnimInstance = Cast<URecallAnimInstance>(Component->GetAnimInstance());
				if (IsValid(AnimInstance))
				{
					if (MovementList.IsValidIndex(EntityIndex))
					{
						const FRecallMovementFragment& MovementFragment = MovementList[EntityIndex];
						AnimInstance->MovementDirection = FVector2f(MovementFragment.MovementDirection.X, MovementFragment.MovementDirection.Y);
					}

					if (BodyList.IsValidIndex(EntityIndex))
					{
						const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
						const FConstRecallPhysicsBodyView Body = PhysicsSystem.GetBody(BodyFragment.BodyHandle);
						if (Body.IsValid())
						{							
							AnimInstance->PhysicsVelocity = Recall::Math::Utils::UnitsPerFrameToPerSecond(
								Body.GetLinearVelocity());
						}
					}

					if (CharacterList.IsValidIndex(EntityIndex))
					{
						const FJPRPhysicsCharacterFragment& CharacterFragment = CharacterList[EntityIndex];
						AnimInstance->bGrounded = CharacterFragment.bIsSupported;
					}
					
					if (ControllerList.IsValidIndex(EntityIndex))
					{
						const FRecallControllerFragment& ControllerFragment = ControllerList[EntityIndex];
						AnimInstance->ControlRotation = ControllerFragment.ControlRotation;
					}
				}

				// We don't want to actually set the delta time for the animation tick to 0 since it will cause a bug with anim notify states
				// triggering their Begin event multiple times.
				Component->SetExternalDeltaTime(AnimationDeltaTime > 0 ? AnimationDeltaTime : DeltaTime);

				const bool StopAnimationTick = DeltaFrame <= 0; // || AnimationFragment.bPauseAnimation
				Component->EnableExternalUpdate(!StopAnimationTick);
			});
		}
	});
}
