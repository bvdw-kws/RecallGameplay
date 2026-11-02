// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#include "System/RecallGameplayCueSubsystem.h"

#include "GameplayCueFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Representation/RecallRepresentationEventSubsystem.h"

#include "System/RecallGameplayCueSnapshot.h"

namespace
{
	FGameplayCueParameters MakeGameplayCueParameters(
		const FRecallGameplayCueParameters& RecallParameters,
		const URecallActorSubsystem& ActorSubsystem)
	{
		FGameplayCueParameters CueParameters;
		CueParameters.NormalizedMagnitude = RecallParameters.NormalizedMagnitude;
		CueParameters.RawMagnitude = RecallParameters.RawMagnitude;
		CueParameters.AggregatedSourceTags = RecallParameters.AggregatedSourceTags;
		CueParameters.AggregatedTargetTags = RecallParameters.AggregatedTargetTags;

		if (RecallParameters.bHasLocation)
		{
			CueParameters.Location = RecallParameters.Location;
		}
		if (RecallParameters.bHasNormal)
		{
			CueParameters.Normal = RecallParameters.Normal;
		}

		CueParameters.Instigator = ActorSubsystem.GetActor<AActor>(RecallParameters.InstigatorActorHandle).Get();
		CueParameters.EffectCauser = ActorSubsystem.GetActor<AActor>(RecallParameters.EffectCauserActorHandle).Get();
		
		return CueParameters;
	}
}

void URecallGameplayCueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<URecallActorSubsystem>();
	Collection.InitializeDependency<URecallRepresentationEventSubsystem>();

	ActorSubsystem = UWorld::GetSubsystem<URecallActorSubsystem>(GetWorld());
	RepresentationEventSubsystem = UWorld::GetSubsystem<URecallRepresentationEventSubsystem>(GetWorld());
}

void URecallGameplayCueSubsystem::Deinitialize()
{
	RemoveAppliedCueStates();
	
	RepresentationEventSubsystem.Reset();
	ActorSubsystem.Reset();
	
	Super::Deinitialize();
}

void URecallGameplayCueSubsystem::Reset()
{
	FScopeLock Lock(&DataGuard);
	ActiveCueInstances.Reset();
	CueInstanceIdGenerator = 0;
	bForceReconcile = true;
}

void URecallGameplayCueSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	FRecallGameplayCueSnapshot Snapshot;
	{
		FScopeLock Lock(&DataGuard);
		ActiveCueInstances.GenerateValueArray(Snapshot.ActiveCueInstances);
		Snapshot.CueInstanceIdGenerator = CueInstanceIdGenerator;
	}

	Snapshot.ActiveCueInstances.Sort([](const FRecallGameplayCueInstance& A, const FRecallGameplayCueInstance& B)
	{
		const int32 ClassNameOrder = A.TargetActorHandle.ClassName.Compare(B.TargetActorHandle.ClassName);
		if (ClassNameOrder != 0)
		{
			return ClassNameOrder < 0;
		}
		if (A.TargetActorHandle.Index != B.TargetActorHandle.Index)
		{
			return A.TargetActorHandle.Index < B.TargetActorHandle.Index;
		}
		if (A.TargetActorHandle.SerialNumber != B.TargetActorHandle.SerialNumber)
		{
			return A.TargetActorHandle.SerialNumber < B.TargetActorHandle.SerialNumber;
		}
		return A.InstanceId < B.InstanceId;
	});

	OutSnapshot.InitializeAs<FRecallGameplayCueSnapshot>(Snapshot);
}

void URecallGameplayCueSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	FScopeLock Lock(&DataGuard);
	
	ActiveCueInstances.Reset();

	if (const FRecallGameplayCueSnapshot* Snapshot = InSnapshot.GetPtr<FRecallGameplayCueSnapshot>())
	{
		CueInstanceIdGenerator = Snapshot->CueInstanceIdGenerator;
		
		for (const FRecallGameplayCueInstance& Instance : Snapshot->ActiveCueInstances)
		{
			if (Instance.TargetActorHandle.IsSet() && Instance.CueTag.IsValid())
			{
				ActiveCueInstances.Add(FRecallGameplayCueInstanceKey(Instance.TargetActorHandle, Instance.InstanceId), Instance);
			}
		}
	}
}

void URecallGameplayCueSubsystem::PostRestore()
{
	FScopeLock Lock(&DataGuard);
	bForceReconcile = true;
}

uint64 URecallGameplayCueSubsystem::AddCue(const FRecallActorHandle& TargetActorHandle,
                                           const FGameplayTag CueTag, const FRecallGameplayCueParameters& Parameters)
{
	if (!TargetActorHandle.IsSet() || !CueTag.IsValid())
	{
		return RECALL_GAMEPLAY_CUE_INVALID_INSTANCE_ID;
	}

	FScopeLock Lock(&DataGuard);
	
	const uint64 InstanceId = ++CueInstanceIdGenerator;

	const FRecallGameplayCueInstanceKey Key(TargetActorHandle, InstanceId);
	FRecallGameplayCueInstance& Instance = ActiveCueInstances.Add(Key);
	Instance.TargetActorHandle = TargetActorHandle;
	Instance.InstanceId = InstanceId;
	Instance.CueTag = CueTag;
	Instance.Parameters = Parameters;
	
	return InstanceId;
}

bool URecallGameplayCueSubsystem::RemoveCue(const FRecallActorHandle& TargetActorHandle, const uint64 InstanceId)
{
	if (!TargetActorHandle.IsSet() || InstanceId == RECALL_GAMEPLAY_CUE_INVALID_INSTANCE_ID)
	{
		return false;
	}

	FScopeLock Lock(&DataGuard);
	const FRecallGameplayCueInstanceKey Key(TargetActorHandle, InstanceId);
	return ActiveCueInstances.Remove(Key) > 0;
}

void URecallGameplayCueSubsystem::ExecuteCue(const FRecallActorHandle& TargetActorHandle, const FGameplayTag CueTag,
	const FRecallGameplayCueParameters& Parameters)
{
	if (!ensureMsgf(CueTag.IsValid(), TEXT("%hs Invalid CueTag"), __FUNCTION__))
	{
		return;
	}

	if (!RepresentationEventSubsystem.IsValid())
	{
		return;
	}

	const TWeakObjectPtr<URecallGameplayCueSubsystem> WeakThis(this);
	RepresentationEventSubsystem->PushEvent([WeakThis, TargetActorHandle, CueTag, Parameters]()
	{
		URecallActorSubsystem* ActorSystem = WeakThis.IsValid() ? WeakThis->ActorSubsystem.Get() : nullptr;
		if (!IsValid(ActorSystem))
		{
			return;
		}

		const TWeakObjectPtr<AActor> TargetActor = ActorSystem->GetActor<AActor>(TargetActorHandle);
		if (!ensureMsgf(TargetActor.IsValid(), TEXT("%hs Target actor is not valid for %s"), __FUNCTION__, *CueTag.ToString()))
		{
			return;
		}

		UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor(
			TargetActor.Get(), CueTag, MakeGameplayCueParameters(Parameters, *ActorSystem));
	});
}

void URecallGameplayCueSubsystem::ReconcileRepresentations()
{
	check(IsInGameThread());

	TArray<FRecallGameplayCueInstance> ActiveInstances;
	bool bShouldForceReconcile = false;
	{
		FScopeLock Lock(&DataGuard);
		ActiveCueInstances.GenerateValueArray(ActiveInstances);
		bShouldForceReconcile = bForceReconcile;
		bForceReconcile = false;
	}

	TMap<FRecallGameplayCueTagKey, FRecallGameplayCueInstance> DesiredCueStates;
	for (const FRecallGameplayCueInstance& Instance : ActiveInstances)
	{
		const FRecallGameplayCueTagKey Key(Instance.TargetActorHandle, Instance.CueTag);
		FRecallGameplayCueInstance* Current = DesiredCueStates.Find(Key);
		if (Current == nullptr || Instance.InstanceId < Current->InstanceId)
		{
			DesiredCueStates.Add(Key, Instance);
		}
	}

	if (!ActorSubsystem.IsValid())
	{
		return;
	}

	// Remove before adding so a pooled actor reused for a different Recall handle cannot retain stale cues.
	for (auto AppliedIt = AppliedCueStates.CreateIterator(); AppliedIt; ++AppliedIt)
	{
		const FRecallGameplayCueTagKey& Key = AppliedIt.Key();
		const FRecallGameplayCueInstance* Desired = DesiredCueStates.Find(Key);
		const TWeakObjectPtr<AActor> CurrentActor = ActorSubsystem->GetActor<AActor>(Key.TargetActorHandle);
		const FRecallGameplayCueAppliedState& Applied = AppliedIt.Value();

		const bool bNeedsRemove = bShouldForceReconcile
			|| Desired == nullptr
			|| !CurrentActor.IsValid()
			|| Applied.Actor.Get() != CurrentActor.Get()
			|| !(Applied.Parameters == Desired->Parameters);
		if (!bNeedsRemove)
		{
			continue;
		}

		if (Applied.Actor.IsValid())
		{
			FGameplayCueParameters CueParameters = MakeGameplayCueParameters(Applied.Parameters, *ActorSubsystem);
			CueParameters.Instigator = Applied.InstigatorActor.Get();
			CueParameters.EffectCauser = Applied.EffectCauserActor.Get();
			
			UGameplayCueFunctionLibrary::RemoveGameplayCueOnActor(
				Applied.Actor.Get(), Key.CueTag, CueParameters);
		}
		AppliedIt.RemoveCurrent();
	}

	for (const TPair<FRecallGameplayCueTagKey, FRecallGameplayCueInstance>& DesiredPair : DesiredCueStates)
	{
		if (AppliedCueStates.Contains(DesiredPair.Key))
		{
			continue;
		}

		const TWeakObjectPtr<AActor> Actor = ActorSubsystem->GetActor<AActor>(DesiredPair.Key.TargetActorHandle);
		if (!Actor.IsValid())
		{
			continue;
		}

		const FGameplayCueParameters CueParameters = MakeGameplayCueParameters(DesiredPair.Value.Parameters, *ActorSubsystem);
		UGameplayCueFunctionLibrary::AddGameplayCueOnActor(Actor.Get(), DesiredPair.Key.CueTag, CueParameters);

		FRecallGameplayCueAppliedState& Applied = AppliedCueStates.FindOrAdd(DesiredPair.Key);
		Applied.Actor = Actor;
		Applied.InstigatorActor = CueParameters.Instigator;
		Applied.EffectCauserActor = CueParameters.EffectCauser;
		Applied.Parameters = DesiredPair.Value.Parameters;
	}
}

void URecallGameplayCueSubsystem::RemoveAppliedCueStates()
{
	check(IsInGameThread());

	for (const TPair<FRecallGameplayCueTagKey, FRecallGameplayCueAppliedState>& AppliedPair : AppliedCueStates)
	{
		if (!AppliedPair.Value.Actor.IsValid())
		{
			continue;
		}
		
		FGameplayCueParameters Parameters;
		if (ActorSubsystem.IsValid())
		{
			Parameters = MakeGameplayCueParameters(AppliedPair.Value.Parameters, *ActorSubsystem);
		}
		Parameters.Instigator = AppliedPair.Value.InstigatorActor.Get();
		Parameters.EffectCauser = AppliedPair.Value.EffectCauserActor.Get();
			
		UGameplayCueFunctionLibrary::RemoveGameplayCueOnActor(
			AppliedPair.Value.Actor.Get(), AppliedPair.Key.CueTag, Parameters);
	}
	
	AppliedCueStates.Reset();
}
