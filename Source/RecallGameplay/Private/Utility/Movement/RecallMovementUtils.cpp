// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Movement/RecallMovementUtils.h"

#include "Kismet/KismetMathLibrary.h"
#include "MassExecutionContext.h"
#include "Physics/Common/JPRPhysicsCommonObjects.h"
#include "Simulation/Attribute/RecallAttributeFragments.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Movement/RecallMovementFragments.h"
#include "Simulation/Movement/RecallMovementTypes.h"
#include "Simulation/Physics/RecallPhysicsCharacterFragments.h"
#include "Utility/Math/RecallMathUtils.h"

namespace Recall::Movement::Utils
{
	
/**
 * Handle movement grounded state for characters.
 * Return false if we should skip the movement processor.
 */
static bool HandleGroundedState(const FRecallMovementContext& MovementContext)
{
	if (MovementContext.CharacterFragmentPtr == nullptr)
	{
		return true;
	}
	
	const bool bIsGrounded = MovementContext.CharacterFragmentPtr->bIsSupported;

	if (MovementContext.GameplayTagsFragmentPtr != nullptr &&
		MovementContext.GameplayTagsFragmentPtr->GameplayTagCountMap.HasTag(State_Grounded) != bIsGrounded)
	{
		bIsGrounded ? MovementContext.GameplayTagsFragmentPtr->GameplayTagCountMap.AddTag(State_Grounded)
			: MovementContext.GameplayTagsFragmentPtr->GameplayTagCountMap.RemoveTag(State_Grounded);
	}

	// Skip movement if our character is not grounded.
	if (MovementContext.MovementConstSharedFragment.MovementSettings.bOnlyMoveOnGround && !bIsGrounded)
	{
		return false;
	}

	return true;
}

static bool CanMove(const FRecallMovementContext& MovementContext)
{
	if (!HandleGroundedState(MovementContext))
	{
		return false;
	}
		
	if (MovementContext.GameplayTagsFragmentPtr != nullptr &&
		MovementContext.GameplayTagsFragmentPtr->GameplayTagCountMap.HasTag(State_IgnoreMovement))
	{
		return false;
	}

	return true;
}

static float GetMovementSpeedModifier(const FRecallMovementContext& MovementContext)
{
	if (MovementContext.AttributeFragmentPtr != nullptr)
	{
		return MovementContext.AttributeFragmentPtr->AttributeSet.GetValue(
			Attribute_MovementSpeedModifier, 1.0f);
	}

	return 1.0f;
}

void HandleMovement(const FRecallMovementContext& MovementContext)
{
	if (!ensure(MovementContext.PhysicsBody.IsValid()) || !CanMove(MovementContext))
	{
		return;
	}

	const FRecallMovementSettings& MovementSettings = MovementContext.MovementConstSharedFragment.MovementSettings;
	const FVector2f& MovementDirection = MovementContext.MovementFragment.MovementDirection;
	const bool bWantsToMove = !MovementDirection.IsNearlyZero();
		
	// Velocity
	if (!MovementContext.ExecutionContext.DoesArchetypeHaveTag<FRecallCrowdMovementTag>())
	{
		const float MovementSpeedModifier = GetMovementSpeedModifier(MovementContext);
		const float MovementSpeed = MovementSettings.GetSpeed();

		FVector2D Velocity2D = MovementContext.PhysicsBody.GetLinearVelocity2D();

		float VelSpeed = 0.0f;
		FVector2D VelDirection = FVector2D::ZeroVector;
		Velocity2D.ToDirectionAndLength(VelDirection, VelSpeed);

		if (bWantsToMove)
		{
			const float Acceleration = MovementSettings.GetAcceleration();
			const FVector2f TargetVel = MovementDirection * MovementSpeed * MovementSpeedModifier;
			
			if (FMath::IsNearlyZero(Acceleration))
			{
				Velocity2D = static_cast<FVector2D>(TargetVel);
			}
			else
			{
				const FVector2f VelDiff = TargetVel - static_cast<FVector2f>(Velocity2D);

				float VelDiffSpeed = 0.0f;
				FVector2f VelDiffDirection = FVector2f::ZeroVector;
				VelDiff.ToDirectionAndLength(VelDiffDirection, VelDiffSpeed);

				Velocity2D += static_cast<FVector2D>(VelDiffDirection) * FMath::Min(
					MovementSettings.GetAcceleration(), VelDiffSpeed);
			}
		}
		else
		{
			const float Deceleration = MovementSettings.GetDeceleration();
			if (FMath::IsNearlyZero(Deceleration))
			{
				Velocity2D = FVector2D::ZeroVector;
			}
			else
			{
				Velocity2D = FMath::Max(0.0f, VelSpeed - Deceleration) * VelDirection;
			}
		}
		
		MovementContext.PhysicsBody.SetLinearVelocity2D(Velocity2D);
	}

	const FVector2D ControlRotationForward = MovementContext.ControllerFragmentPtr != nullptr ?
		static_cast<FVector2D>(MovementContext.ControllerFragmentPtr->ControlRotation.Quaternion().GetForwardVector()) : static_cast<FVector2D>(MovementDirection);

	// Rotation
	FVector ForwardVec = FVector::ZeroVector;

	if (EnumHasAnyFlags(MovementSettings.RotateAxis , ERecallMovementRotationAxis::X))
	{
		ForwardVec.X = MovementSettings.bFaceControlRotation ? ControlRotationForward.X : MovementDirection.X;
	}
	if (EnumHasAnyFlags(MovementSettings.RotateAxis , ERecallMovementRotationAxis::Y))
	{
		ForwardVec.Y = MovementSettings.bFaceControlRotation ? ControlRotationForward.Y : MovementDirection.Y;
	}
			
	if (bWantsToMove && !ForwardVec.IsNearlyZero())
	{
		FQuat Rotation = FQuat::Identity;
		MovementContext.PhysicsBody.GetRotation(Rotation);

		ForwardVec = UKismetMathLibrary::VInterpTo(Rotation.GetForwardVector(), ForwardVec,
			MovementContext.ExecutionContext.GetDeltaTimeSeconds(), MovementSettings.RotateInterpSpeed);
		
		const FVector RightVec = FVector::CrossProduct(FVector::UpVector, ForwardVec);
		const FRotator NewRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVec, RightVec, FVector::UpVector);


		MovementContext.PhysicsBody.SetRotation(NewRotation);
	}
}

static bool CanJump(const FRecallMovementContext& MovementContext)
{
	if (MovementContext.CharacterFragmentPtr != nullptr && !MovementContext.CharacterFragmentPtr->bIsSupported)
	{
		return false;
	}

	return true;
}

void HandleJump(const FRecallMovementContext& MovementContext)
{
	const bool bWantsToJump = static_cast<bool>(MovementContext.MovementFragment.bWantsToJump);
	MovementContext.MovementFragment.bWantsToJump = false;
			
	if (bWantsToJump && CanJump(MovementContext) && ensure(MovementContext.PhysicsBody.IsValid()))
	{
		const float JumpZVelocityCentimetersPerSecond = MovementContext.MovementConstSharedFragment.MovementSettings.JumpZVelocityCentimetersPerSecond;
		const float JumpZVelocityCentimetersPerFrame = Recall::Math::Utils::UnitsPerSecondToPerFrame(JumpZVelocityCentimetersPerSecond);
		
		MovementContext.PhysicsBody.SetLinearZVelocity(JumpZVelocityCentimetersPerFrame);
	}
}

} // namespace Recall::Movement::Utils
