// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "System/Actor/RecallActorTypes.h"
#include "Templates/SubclassOf.h"

#include "RecallGameplayEffectInstance.generated.h"

class URecallGameplayEffect;

USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectInstance
{
	GENERATED_BODY()

	/**
	 * Entity owning this gameplay effect instance.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle OwnerEntity;

	/**
	 * Class of the gameplay effect for this instance.
	 */
	UPROPERTY(VisibleAnywhere)
	TSubclassOf<URecallGameplayEffect> EffectClass;

	/**
	 * Time elapsed for this effect.
	 */
	UPROPERTY(VisibleAnywhere)
	double TimeSeconds = 0.0f;

	/**
	 * Duration computed when the effect was added (base duration with duration operations applied).
	 * 0 means that it is infinite.
	 */
	UPROPERTY(VisibleAnywhere)
	float ComputedDuration = 0.0f;

	/**
	 * Handles to keep track of the attribute modifiers added by this effect instance so they can be removed.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<uint32> AttributeModHandles;

	/** Representation that received this effect's persistent cues. */
	UPROPERTY(VisibleAnywhere)
	FRecallActorHandle GameplayCueTargetActorHandle;

	/** Stable IDs used to remove the exact cue contributions created by this instance. */
	UPROPERTY(VisibleAnywhere)
	TArray<uint64> GameplayCueInstanceIds;

	/** Cue tags counted on the target while this effect instance is active. */
	UPROPERTY(VisibleAnywhere)
	TArray<FGameplayTag> GameplayCueTags;
};
