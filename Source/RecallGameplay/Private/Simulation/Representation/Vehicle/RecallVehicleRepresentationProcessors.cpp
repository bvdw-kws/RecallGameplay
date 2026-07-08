// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallVehicleRepresentationProcessors.h"

#include "Actor/Vehicle/Interface/RecallPhysicsVehicleActorInterface.h"
#include "MassExecutionContext.h"
#include "Physics/Vehicle/JPRPhysicsVehicleObject.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Physics/RecallPhysicsVehicleFragments.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Physics/RecallPhysicsSubsystem.h"

//----------------------------------------------------------------------//
// URecallVehicleRepresentationProcessor
//----------------------------------------------------------------------//
URecallVehicleRepresentationProcessor::URecallVehicleRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallVehicleRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallVehicleRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallPhysicsVehicleFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadOnly);
}

void URecallVehicleRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_VehicleRepresentation_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const URecallPhysicsSubsystem& PhysicsSystem = Context.GetSubsystemChecked<URecallPhysicsSubsystem>();
		const URecallActorSubsystem& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();
		
		const TConstArrayView<FRecallPhysicsVehicleFragment> VehicleList = Context.GetFragmentView<FRecallPhysicsVehicleFragment>();
		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];			
			const TWeakObjectPtr<AActor> Actor = ActorSystem.GetActor(ActorFragment.ActorHandle);
			if (!Actor.IsValid() || !Actor->GetClass()->ImplementsInterface(URecallPhysicsVehicleActorInterface::StaticClass()))
			{
				continue;
			}

			const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
			const FConstRecallPhysicsBodyView Body = PhysicsSystem.GetBody(BodyFragment.BodyHandle);
			const TWeakPtr<const FJPRPhysicsVehicleBody> Vehicle = StaticCastWeakPtr<const FJPRPhysicsVehicleBody>(Body.GetBody());
			if (!Vehicle.IsValid())
			{
				continue;
			}
			
			const FRecallPhysicsVehicleFragment& VehicleFragment = VehicleList[EntityIndex];			
			IRecallPhysicsVehicleActorInterface::Execute_SetDriverInput(Actor.Get(),
				VehicleFragment.Forward, VehicleFragment.Right, VehicleFragment.Brake, VehicleFragment.HandBrake);

			FRecallVehicleRepresentationInfo RepresentationInfo;
			RepresentationInfo.SpeedKilometersPerHour = Vehicle.Pin()->GetSpeedKilometersPerHour(Context.GetWorld());
			RepresentationInfo.bHasRearContact = Vehicle.Pin()->HasRearWheelsContact();
			
			IRecallPhysicsVehicleActorInterface::Execute_SetVehicleRepresentationInfo(Actor.Get(), RepresentationInfo);
		}
	});
}
