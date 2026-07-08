// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallCharacterEvaluators.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Physics/Character/JPRPhysicsCharacterVirtualObject.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "System/Physics/RecallPhysicsSubsystem.h"

//----------------------------------------------------------------------//
// FRecallCharacterEvaluator
//----------------------------------------------------------------------//
bool FRecallCharacterEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BodyFragmentHandle);
	Linker.LinkExternalData(PhysicsSystemHandle);
	return true;
}

void FRecallCharacterEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
}

void FRecallCharacterEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	const URecallPhysicsSubsystem& PhysicsSystem = Context.GetExternalData(PhysicsSystemHandle);
	const FRecallPhysicsBodyFragment& BodyFragment = Context.GetExternalData(BodyFragmentHandle);

	const TWeakPtr<const FJPRPhysicsCharacterVirtualBody> CharacterVirtualBody = PhysicsSystem
		.GetBody(BodyFragment.BodyHandle).GetBody<FJPRPhysicsCharacterVirtualBody>();
	if (CharacterVirtualBody.IsValid())
	{
		InstanceData.bStanding = CharacterVirtualBody.Pin()->IsStanding();
	}
}
