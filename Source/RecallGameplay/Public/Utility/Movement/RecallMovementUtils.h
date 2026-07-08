// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Physics/RecallPhysicsObjects.h"

struct FRecallMovementContext
{
	struct FMassExecutionContext& ExecutionContext;
	struct FRecallMovementFragment& MovementFragment;
	const struct FRecallMovementSharedFragment& MovementConstSharedFragment;
	const FRecallPhysicsBodyView& PhysicsBody;
	const struct FJPRPhysicsCharacterFragment* CharacterFragmentPtr = nullptr;
	struct FRecallGameplayTagFragment* const GameplayTagsFragmentPtr = nullptr;
	const struct FRecallAttributeFragment* const AttributeFragmentPtr = nullptr;
	const struct FRecallControllerFragment* const ControllerFragmentPtr = nullptr;
};

/**
 * Helper functions to handle movement.
 * These functions must not rely on external data since movement is parallelized.
 */
namespace Recall::Movement::Utils
{

RECALLGAMEPLAY_API extern void HandleMovement(const FRecallMovementContext& MovementContext);
RECALLGAMEPLAY_API extern void HandleJump(const FRecallMovementContext& MovementContext);
	
} // namespace Recall::Movement::Utils
