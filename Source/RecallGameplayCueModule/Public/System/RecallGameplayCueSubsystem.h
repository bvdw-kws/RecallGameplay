// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Mass/ExternalSubsystemTraits.h"
#include "System/RecallGameplayCueTypes.h"

#include "RecallGameplayCueSubsystem.generated.h"

/**
 * Keeps rollback-restorable cue intent separate from the transient actor-side cue state.
 * AddCue/RemoveCue/ExecuteCue are safe to call from Recall simulation workers.
 */
UCLASS()
class RECALLGAMEPLAYCUEMODULE_API URecallGameplayCueSubsystem : public UWorldSubsystem, public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	/**
	 * Add a persistent cue contribution and return its generated non-zero ID for later removal.
	 * Returns zero when the target handle or cue tag is invalid. Contributions with the same target
	 * and tag share one UE cue lifecycle; the lowest generated ID supplies its parameters.
	 */
	uint64 AddCue(const FRecallActorHandle& TargetActorHandle, FGameplayTag CueTag,
		const FRecallGameplayCueParameters& Parameters = FRecallGameplayCueParameters());

	/** Remove one persistent cue contribution. The UE cue is removed when its last contribution ends. */
	bool RemoveCue(const FRecallActorHandle& TargetActorHandle, uint64 InstanceId);

	/** Queue a one-shot Execute cue for the representation event subsystem's confirmed-frame dispatch. */
	void ExecuteCue(const FRecallActorHandle& TargetActorHandle, FGameplayTag CueTag,
		const FRecallGameplayCueParameters& Parameters = FRecallGameplayCueParameters());

	// UWorldSubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// IRecallSimulationReactSystemInterface implementation
	virtual void Reset() override;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override;
	virtual void PostRestore() override;

private:
	friend class URecallGameplayCueRepresentationProcessor;

	UPROPERTY(Transient)
	TWeakObjectPtr<class URecallRepresentationEventSubsystem> RepresentationEventSubsystem;

	UPROPERTY(Transient)
	TWeakObjectPtr<class URecallActorSubsystem> ActorSubsystem;

	FCriticalSection DataGuard;
	TMap<FRecallGameplayCueInstanceKey, FRecallGameplayCueInstance> ActiveCueInstances;
	uint64 CueInstanceIdGenerator = 0;
	bool bForceReconcile = false;

	// Game-thread-only representation cache. Gameplay Cue state itself is never snapshotted.
	TMap<FRecallGameplayCueTagKey, FRecallGameplayCueAppliedState> AppliedCueStates;

	void ReconcileRepresentations();
	void RemoveAppliedCueStates();
};

template<>
struct TMassExternalSubsystemTraits<URecallGameplayCueSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
