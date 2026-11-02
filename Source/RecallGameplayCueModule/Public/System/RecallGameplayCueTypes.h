// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "System/Actor/RecallActorTypes.h"

#include "RecallGameplayCueTypes.generated.h"

RECALLGAMEPLAYCUEMODULE_API DECLARE_LOG_CATEGORY_EXTERN(LogRecallGameplayCue, Log, All);

#define RECALL_GAMEPLAY_CUE_INVALID_INSTANCE_ID 0

/** Serializable cue payload. Actor references are stored as Recall handles, never UObject pointers. */
USTRUCT()
struct RECALLGAMEPLAYCUEMODULE_API FRecallGameplayCueParameters
{
	GENERATED_BODY()

	UPROPERTY()
	float NormalizedMagnitude = 0.f;

	UPROPERTY()
	float RawMagnitude = 0.f;

	UPROPERTY()
	bool bHasLocation = false;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	bool bHasNormal = false;

	UPROPERTY()
	FVector Normal = FVector::ZeroVector;

	UPROPERTY()
	FRecallActorHandle InstigatorActorHandle;

	UPROPERTY()
	FRecallActorHandle EffectCauserActorHandle;

	UPROPERTY()
	FGameplayTagContainer AggregatedSourceTags;

	UPROPERTY()
	FGameplayTagContainer AggregatedTargetTags;

	bool operator==(const FRecallGameplayCueParameters& Other) const
	{
		return NormalizedMagnitude == Other.NormalizedMagnitude
			&& RawMagnitude == Other.RawMagnitude
			&& bHasLocation == Other.bHasLocation
			&& Location == Other.Location
			&& bHasNormal == Other.bHasNormal
			&& Normal == Other.Normal
			&& InstigatorActorHandle == Other.InstigatorActorHandle
			&& EffectCauserActorHandle == Other.EffectCauserActorHandle
			&& AggregatedSourceTags == Other.AggregatedSourceTags
			&& AggregatedTargetTags == Other.AggregatedTargetTags;
	}
};

/** One active cue contribution owned by a deterministic simulation instance. */
USTRUCT()
struct RECALLGAMEPLAYCUEMODULE_API FRecallGameplayCueInstance
{
	GENERATED_BODY()

	UPROPERTY()
	FRecallActorHandle TargetActorHandle;

	/** Caller supplied, stable and unique for this target while this contribution is active. Zero is invalid. */
	UPROPERTY()
	uint64 InstanceId = RECALL_GAMEPLAY_CUE_INVALID_INSTANCE_ID;

	UPROPERTY()
	FGameplayTag CueTag;

	UPROPERTY()
	FRecallGameplayCueParameters Parameters;
};

/** Key for a logical simulation-owned cue contribution. */
struct RECALLGAMEPLAYCUEMODULE_API FRecallGameplayCueInstanceKey
{
	FRecallGameplayCueInstanceKey() = default;
	FRecallGameplayCueInstanceKey(const FRecallActorHandle& InActorHandle, uint64 InInstanceId)
		: TargetActorHandle(InActorHandle), InstanceId(InInstanceId)
	{
	}

	FRecallActorHandle TargetActorHandle;
	uint64 InstanceId = 0;

	bool operator==(const FRecallGameplayCueInstanceKey& Other) const
	{
		return TargetActorHandle == Other.TargetActorHandle && InstanceId == Other.InstanceId;
	}

	friend uint32 GetTypeHash(const FRecallGameplayCueInstanceKey& Key)
	{
		return HashCombine(GetTypeHash(Key.TargetActorHandle), GetTypeHash(Key.InstanceId));
	}
};

/** Key for one UE cue lifecycle on a target representation. */
struct RECALLGAMEPLAYCUEMODULE_API FRecallGameplayCueTagKey
{
	FRecallGameplayCueTagKey() = default;
	FRecallGameplayCueTagKey(const FRecallActorHandle& InActorHandle, const FGameplayTag& InCueTag)
		: TargetActorHandle(InActorHandle), CueTag(InCueTag)
	{
	}

	FRecallActorHandle TargetActorHandle;
	FGameplayTag CueTag;

	bool operator==(const FRecallGameplayCueTagKey& Other) const
	{
		return TargetActorHandle == Other.TargetActorHandle && CueTag == Other.CueTag;
	}

	friend uint32 GetTypeHash(const FRecallGameplayCueTagKey& Key)
	{
		return HashCombine(GetTypeHash(Key.TargetActorHandle), GetTypeHash(Key.CueTag));
	}
};

/** Transient render-side state. These weak pointers are deliberately excluded from snapshots. */
struct RECALLGAMEPLAYCUEMODULE_API FRecallGameplayCueAppliedState
{
	TWeakObjectPtr<AActor> Actor;
	TWeakObjectPtr<AActor> InstigatorActor;
	TWeakObjectPtr<AActor> EffectCauserActor;
	FRecallGameplayCueParameters Parameters;
};
