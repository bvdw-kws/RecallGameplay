// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallCharacterTasks.h"

#include "Engine/World.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Physics/Character/JPRPhysicsCharacterVirtualObject.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/Movement/RecallMovementFragments.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

//----------------------------------------------------------------------//
// FRecallCharacterLocomotionTask
//----------------------------------------------------------------------//
bool FRecallCharacterLocomotionTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MovementFragmentHandle);
	Linker.LinkExternalData(ControllerFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallCharacterLocomotionTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	return Super::EnterState(Context, Transition);
}

void FRecallCharacterLocomotionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FRecallMovementFragment& MovementFragment = Context.GetExternalData(MovementFragmentHandle);
	MovementFragment.Stop();
	
	return Super::ExitState(Context, Transition);
}

EStateTreeRunStatus FRecallCharacterLocomotionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FRecallControllerFragment& ControllerFragment = Context.GetExternalData(ControllerFragmentHandle);	
	FRecallMovementFragment& MovementFragment = Context.GetExternalData(MovementFragmentHandle);

	GetMovementControl(Context, MovementFragment.MovementDirection, ControllerFragment.ControlRotation);
	
	return Super::Tick(Context, DeltaTime);
}

void FRecallCharacterLocomotionTask::GetMovementControl(FStateTreeExecutionContext& Context,
	FVector2f& OutMovementDirection, FRotator& OutControlRotation) const
{
	const float FixedDeltaTime = Recall::Simulation::Utils::GetDilatedFixedDeltaTime(Context.GetWorld());	
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	OutControlRotation.Yaw += InstanceData.LookAxis.X * LookSpeed * FixedDeltaTime;
	OutControlRotation.Pitch += InstanceData.LookAxis.Y * LookSpeed * FixedDeltaTime;

	// TODO: Project on Z-Plane
	const FVector ForwardVector = OutControlRotation.Quaternion().GetForwardVector();
	const FVector RightVector = OutControlRotation.Quaternion().GetRightVector();

	const FVector ForwardMovement = ForwardVector * InstanceData.MovementAxis.Y;
	const FVector LateralMovement = RightVector * InstanceData.MovementAxis.X;

	const FVector Movement = ForwardMovement + LateralMovement;

	OutMovementDirection = FVector2f(Movement.X, Movement.Y);
}

//----------------------------------------------------------------------//
// FRecallCharacterJumpTask
//----------------------------------------------------------------------//
bool FRecallCharacterJumpTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MovementFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallCharacterJumpTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FRecallMovementFragment& MovementFragment = Context.GetExternalData(MovementFragmentHandle);
	MovementFragment.bWantsToJump = true;

	return EStateTreeRunStatus::Succeeded;
}

void FRecallCharacterJumpTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{	
	return Super::ExitState(Context, Transition);
}

EStateTreeRunStatus FRecallCharacterJumpTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return Super::Tick(Context, DeltaTime);
}

//----------------------------------------------------------------------//
// FRecallCharacterVirtualStanceTask
//----------------------------------------------------------------------//
bool FRecallCharacterVirtualStanceTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BodyFragmentHandle);
	Linker.LinkExternalData(PhysicsSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallCharacterVirtualStanceTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FRecallPhysicsBodyFragment& BodyFragment = Context.GetExternalData(BodyFragmentHandle);
	URecallPhysicsSubsystem& PhysicsSystem = Context.GetExternalData(PhysicsSystemHandle);

	const FRecallPhysicsBodyView BodyView = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);
	const TWeakPtr<FJPRPhysicsCharacterVirtualBody> CharacterVirtualBody = BodyView.GetBody<FJPRPhysicsCharacterVirtualBody>();
	if (!CharacterVirtualBody.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (CharacterVirtualBody.Pin()->SetStance(InstanceData.bStand))
	{
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
}
