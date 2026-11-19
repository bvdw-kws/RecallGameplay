// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallCharacterConditions.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Physics/Character/JPRPhysicsCharacterVirtualObject.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "System/Physics/RecallPhysicsSubsystem.h"

//----------------------------------------------------------------------//
// FRecallCharacterStandingCondition
//----------------------------------------------------------------------//
bool FRecallCharacterStandingCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BodyFragmentHandle);
	Linker.LinkExternalData(PhysicsSystemHandle);
	return true;
}

bool FRecallCharacterStandingCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const URecallPhysicsSubsystem& PhysicsSystem = Context.GetExternalData(PhysicsSystemHandle);
	const FRecallPhysicsBodyFragment& BodyFragment = Context.GetExternalData(BodyFragmentHandle);

	bool bResult = true;

	const FConstRecallPhysicsBodyView BodyView = PhysicsSystem.GetBody(BodyFragment.BodyHandle);
	const TWeakPtr<const FJPRPhysicsCharacterVirtualBody> CharacterVirtualBody = BodyView.GetBody<FJPRPhysicsCharacterVirtualBody>();
	if (CharacterVirtualBody.IsValid())
	{
		bResult = CharacterVirtualBody.Pin()->IsStanding();
	}

	return bResult != bInvert;
}
