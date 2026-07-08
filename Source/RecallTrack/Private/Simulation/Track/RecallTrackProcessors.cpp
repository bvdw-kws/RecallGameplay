// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallTrackProcessors.h"

#include "MassExecutionContext.h"
#include "Actor/RecallTrackEntityActor.h"
#include "Physics/Common/JPRPhysicsCommonShapeTypes.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Track/RecallTrackFragments.h" 
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "System/Track/RecallTrackSubsystem.h"
#include "Track/RecallTrackTypes.h"
#include "Utility/Physics/RecallPhysicsUtils.h"

//----------------------------------------------------------------------//
// URecallTrackFragmentConstructor
//----------------------------------------------------------------------//
URecallTrackFragmentConstructor::URecallTrackFragmentConstructor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallTrackFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallTrackFragmentConstructor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallTrackFragmentConstructor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallTrackFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FRecallTrackConstSharedFragment>();
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallTrackSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallTrackFragmentConstructor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_TrackInit_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const URecallTrackSubsystem& TrackSystem = Context.GetSubsystemChecked<URecallTrackSubsystem>();
		URecallPhysicsSubsystem& PhysicsSystem = Context.GetMutableSubsystemChecked<URecallPhysicsSubsystem>();

		const FRecallTrackConstSharedFragment& TrackConstSharedFragment = Context.GetConstSharedFragment<FRecallTrackConstSharedFragment>();
		
		const TArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetMutableFragmentView<FRecallPhysicsBodyFragment>();
		const TConstArrayView<FRecallTrackFragment> TrackList = Context.GetFragmentView<FRecallTrackFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallTrackFragment& TrackFragment = TrackList[EntityIndex];
			const TWeakObjectPtr<ARecallTrackEntityActor> TrackActor = TrackSystem.GetTrackActor(TrackFragment.TrackAssetName);
			if (!ensure(TrackActor.IsValid()))
			{
				continue;
			}

			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);

			FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
			FJPRPhysicsStaticCompoundShape StaticCompoundShape;
			
			for (const FRecallTrackSegment& TrackSegment : TrackActor->GetTrackSegments())
			{
				FJPRPhysicsStaticCompoundSubShape& MeshShape = StaticCompoundShape.SubShapes.AddDefaulted_GetRef();
				MeshShape.MeshShape.Vertices = TrackSegment.Vertices;
				MeshShape.MeshShape.Triangles = TrackSegment.Triangles;
				MeshShape.MeshShape.MeshShapeSettings = TrackConstSharedFragment.MeshShapeSettings;
				MeshShape.Position = TrackSegment.Location;
				MeshShape.Rotation = TrackSegment.Rotation;
			}

			BodyFragment.BodyHandle = PhysicsSystem.CreateShape(
				Entity, StaticCompoundShape, TrackConstSharedFragment.Params);
			
			Recall::Physics::Utils::InitializePhysicsBody(Context, Entity,
				PhysicsSystem, BodyFragment, TrackConstSharedFragment.Params);
		}
	});
}
