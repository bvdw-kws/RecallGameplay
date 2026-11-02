// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGameplayEffectContainer.h"

#include "MassEntityView.h"
#include "RecallGameplayEffect.h"
#include "RecallGameplayEffectContext.h"
#include "Engine/World.h"
#include "MassEntityManager.h"
#include "Simulation/Attribute/RecallAttributeFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "System/RecallGameplayCueSubsystem.h"

void FRecallGameplayEffectContainer::Initialize(const FMassEntityHandle& InOwnerEntity)
{
	OwnerEntity = InOwnerEntity;

	checkf(EffectInstanceMap.IsEmpty(),
		TEXT("%hs Effect has been added before container was initialized"), __FUNCTION__);
}

FRecallGameplayEffectHandle FRecallGameplayEffectContainer::AddEffect(
	const FRecallGameplayEffectContext& Context, const TSubclassOf<URecallGameplayEffect>& Effect)
{
	FRecallGameplayEffectHandle NewHandle = FRecallGameplayEffectHandle::Invalid();
	if (!Effect)
	{
		return NewHandle;
	}
	
	const URecallGameplayEffect* EffectCDO = Effect.GetDefaultObject();
	if (!EffectCDO)
	{
		return NewHandle;
	}

	FRecallGameplayEffectInstance* Instance = nullptr;
	
	if (EffectCDO->DurationPolicy != ERecallGameplayEffectDurationType::Instant)
	{
		HandleEffectStack(Context, Effect);
	
		NewHandle = FRecallGameplayEffectHandle::Make(++SerialNumberGenerator);
		Instance = &EffectInstanceMap.Add(NewHandle);
		Instance->OwnerEntity = Context.OwnerEntity;
		Instance->EffectClass = Effect;
	}

	AddEffect_Internal(Context, *EffectCDO, Instance);
	
	return NewHandle;
}

void FRecallGameplayEffectContainer::AddEffects(const FRecallGameplayEffectContext& Context,
	const TArray<TSubclassOf<URecallGameplayEffect>>& Effects)
{
	for (const TSubclassOf<URecallGameplayEffect>& Effect : Effects)
	{
		AddEffect(Context, Effect);
	}
}

bool FRecallGameplayEffectContainer::RemoveEffect(const FRecallGameplayEffectContext& Context,
                                                    const FRecallGameplayEffectHandle& Handle)
{
	if (!Handle.IsSet())
	{
		return false;
	}
	
	FRecallGameplayEffectInstance Instance;
	if (EffectInstanceMap.RemoveAndCopyValue(Handle, Instance))
	{
		RemoveEffect_Internal(Context, Instance);
		return true;
	}
	return false;
}

void FRecallGameplayEffectContainer::RemoveEffectsByClasses(const FRecallGameplayEffectContext& Context,
	const TArray<TSubclassOf<URecallGameplayEffect>>& Effects)
{
	for (const TSubclassOf<URecallGameplayEffect>& Effect : Effects)
	{
		RemoveEffectsByClass(Context, Effect);
	}
}

bool FRecallGameplayEffectContainer::RemoveEffectsByClass(const FRecallGameplayEffectContext& Context,
	const TSubclassOf<URecallGameplayEffect>& Effect)
{
	if (!Effect)
	{
		return false;
	}
	
	TArray<FRecallGameplayEffectHandle> Handles;
	EffectInstanceMap.GetKeys(Handles);

	bool bResult = false;
	
	for (const FRecallGameplayEffectHandle& Handle : Handles)
	{
		const FRecallGameplayEffectInstance& Instance = EffectInstanceMap[Handle];
		if (EffectInstanceMap[Handle].EffectClass == Effect)
		{
			RemoveEffect_Internal(Context, Instance);
			EffectInstanceMap.Remove(Handle);
		}
	}

	return bResult;
}

bool FRecallGameplayEffectContainer::RemoveEffectsByTag(const FRecallGameplayEffectContext& Context,
	const FGameplayTag& EffectTag)
{
	if (!EffectTag.IsValid())
	{
		return false;
	}
	
	TArray<FRecallGameplayEffectHandle> Handles;
	EffectInstanceMap.GetKeys(Handles);

	bool bResult = false;
	
	for (const FRecallGameplayEffectHandle& Handle : Handles)
	{
		const FRecallGameplayEffectInstance& Instance = EffectInstanceMap[Handle];
		const URecallGameplayEffect* EffectCDO = Instance.EffectClass.GetDefaultObject();
		if (IsValid(EffectCDO) && EffectCDO->EffectTag == EffectTag)
		{
			RemoveEffect_Internal(Context, Instance);
			EffectInstanceMap.Remove(Handle);
		}
	}

	return bResult;
}

void FRecallGameplayEffectContainer::HandleEffectStack(const FRecallGameplayEffectContext& Context,
	const TSubclassOf<URecallGameplayEffect>& Effect)
{
	const URecallGameplayEffect* EffectCDO = Effect.GetDefaultObject();
	if (!EffectCDO || !EffectCDO->bUseStack)
	{
		return;
	}
	
	FMassEntityHandle EffectOwnerEntity;
	
	switch (EffectCDO->StackRule)
	{
	case ERecallGameplayEffectStackRule::Shared:
		break;

	case ERecallGameplayEffectStackRule::PerOwner:
		EffectOwnerEntity = Context.OwnerEntity;
		break;

	default:
		unimplemented()
		break;
	}

	int32 OwnedEffectCount = 0;
	const FRecallGameplayEffectHandle OldestEffectHandle = GetOldestEffectHandle(
		Effect, EffectOwnerEntity, OwnedEffectCount);

	if (OwnedEffectCount < EffectCDO->StackLimit)
	{
		return;
	}
	
	RemoveEffect(Context, OldestEffectHandle);
}

FRecallGameplayEffectHandle FRecallGameplayEffectContainer::GetOldestEffectHandle(
	const TSubclassOf<URecallGameplayEffect>& Effect,	const FMassEntityHandle& EffectOwnerEntity,
	int32& OutEffectOwnedCount) const
{	
	const TArray<FRecallGameplayEffectHandle> EffectHandles = GetAllEffectHandles(Effect, EffectOwnerEntity);
	OutEffectOwnedCount = EffectHandles.Num();
	
	if (OutEffectOwnedCount == 0)
	{
		return FRecallGameplayEffectHandle();
	}

	FRecallGameplayEffectHandle OldestEffectHandle = EffectHandles[0];

	for (int32 HandleIndex = 1; HandleIndex < EffectHandles.Num(); ++HandleIndex)
	{
		if (EffectHandles[HandleIndex].SerialNumber < OldestEffectHandle.SerialNumber)
		{
			OldestEffectHandle = EffectHandles[HandleIndex];
		}
	}

	return OldestEffectHandle;
}

void FRecallGameplayEffectContainer::AddEffect_Internal(const FRecallGameplayEffectContext& Context,
	const URecallGameplayEffect& EffectCDO, FRecallGameplayEffectInstance* Instance) const
{
	const FMassEntityView OwnerView(Context.EntityManager, OwnerEntity);
	if (!OwnerView.IsValid())
	{
		return;
	}

	const bool bInstant = EffectCDO.DurationPolicy == ERecallGameplayEffectDurationType::Instant;

	// Gameplay tags
	if (!EffectCDO.GameplayTagsToAdd.IsEmpty())
	{
		if (auto* GameplayTagFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
		{
			GameplayTagFragmentPtr->GameplayTagCountMap.AddTags(EffectCDO.GameplayTagsToAdd);
		}
	}

	// Attribute modifiers
	if (!EffectCDO.AttributeMods.IsEmpty())
	{
		if (auto* AttributeFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallAttributeFragment>())
		{
			for (const TSubclassOf<URecallAttributeMod>& AttributeMod : EffectCDO.AttributeMods)
			{
				const uint32 AttributeModHandle = AttributeFragmentPtr->AttributeSet.AddMod(AttributeMod, bInstant);
				if (AttributeModHandle != RECALL_ATTRIBUTE_MOD_HANDLE_NULL && ensure(Instance))
				{
					Instance->AttributeModHandles.Add(AttributeModHandle);
				}
			}
		}
	}

	AddGameplayCue_Internal(Context, EffectCDO, OwnerView, Instance, bInstant);
}

void FRecallGameplayEffectContainer::AddGameplayCue_Internal(const FRecallGameplayEffectContext& Context,
	const URecallGameplayEffect& EffectCDO, const FMassEntityView& OwnerView,
	FRecallGameplayEffectInstance* Instance, bool bInstant) const
{
	if (EffectCDO.GameplayCues.IsEmpty())
	{
		return;
	}

	// Persistent cue tags are also owned tags, matching GAS's active cue tag behavior.
	if (!bInstant && Instance != nullptr)
	{
		if (FRecallGameplayTagFragment* GameplayTagFragmentPtr =
			OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
		{
			for (const FRecallGameplayEffectCue& EffectCue : EffectCDO.GameplayCues)
			{
				GameplayTagFragmentPtr->GameplayTagCountMap.AddTags(EffectCue.GameplayCueTags);
				Instance->GameplayCueTags.Append(EffectCue.GameplayCueTags.GetGameplayTagArray());
			}
		}
	}

	URecallGameplayCueSubsystem* GameplayCueSubsystem = UWorld::GetSubsystem<URecallGameplayCueSubsystem>(Context.EntityManager.GetWorld());
	if (!IsValid(GameplayCueSubsystem))
	{
		return;
	}

	const FRecallActorRepresentationFragment* TargetRepresentationFragment =
		OwnerView.GetFragmentDataPtr<FRecallActorRepresentationFragment>();
	const FRecallActorHandle TargetActorHandle = TargetRepresentationFragment
		? TargetRepresentationFragment->ActorHandle
		: FRecallActorHandle::Invalid();

	FRecallGameplayCueParameters BaseCueParameters;
	
	const FMassEntityView InstigatorView(Context.EntityManager, Context.OwnerEntity);
	if (InstigatorView.IsValid())
	{
		if (const FRecallGameplayTagFragment* InstigatorTags =
			InstigatorView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
		{
			BaseCueParameters.AggregatedSourceTags = InstigatorTags->GameplayTagCountMap.GetTags();
		}
		if (const FRecallActorRepresentationFragment* InstigatorRepresentation =
			InstigatorView.GetFragmentDataPtr<FRecallActorRepresentationFragment>())
		{
			BaseCueParameters.InstigatorActorHandle = InstigatorRepresentation->ActorHandle;
			// Recall currently has a single source entity in its effect context, so use it as the effect causer too.
			BaseCueParameters.EffectCauserActorHandle = BaseCueParameters.InstigatorActorHandle;
		}
	}
	
	if (const FRecallGameplayTagFragment* TargetTags =
		OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
	{
		BaseCueParameters.AggregatedTargetTags = TargetTags->GameplayTagCountMap.GetTags();
	}

	if (!bInstant && Instance != nullptr)
	{
		Instance->GameplayCueTargetActorHandle = TargetActorHandle;
	}

	for (const FRecallGameplayEffectCue& EffectCue : EffectCDO.GameplayCues)
	{
		FRecallGameplayCueParameters CueParameters = BaseCueParameters;
		CueParameters.NormalizedMagnitude = EffectCue.NormalizedMagnitude;
		CueParameters.RawMagnitude = EffectCue.RawMagnitude;

		for (const FGameplayTag& CueTag : EffectCue.GameplayCueTags)
		{
			if (bInstant)
			{
				GameplayCueSubsystem->ExecuteCue(TargetActorHandle, CueTag, CueParameters);
			}
			else if (Instance != nullptr)
			{
				const uint64 CueInstanceId = GameplayCueSubsystem->AddCue(TargetActorHandle, CueTag, CueParameters);
				if (CueInstanceId != 0)
				{
					Instance->GameplayCueInstanceIds.Add(CueInstanceId);
				}
			}
		}
	}
}

void FRecallGameplayEffectContainer::RemoveEffect_Internal(const FRecallGameplayEffectContext& Context,
	const FRecallGameplayEffectInstance& Instance) const
{
	if (Instance.GameplayCueTargetActorHandle.IsSet() && !Instance.GameplayCueInstanceIds.IsEmpty())
	{
		if (UWorld* World = Context.EntityManager.GetWorld())
		{
			if (URecallGameplayCueSubsystem* GameplayCueSubsystem =
				UWorld::GetSubsystem<URecallGameplayCueSubsystem>(World))
			{
				for (const uint64 CueInstanceId : Instance.GameplayCueInstanceIds)
				{
					GameplayCueSubsystem->RemoveCue(Instance.GameplayCueTargetActorHandle, CueInstanceId);
				}
			}
		}
	}

	const FMassEntityView OwnerView(Context.EntityManager, OwnerEntity);
	if (!OwnerView.IsValid())
	{
		return;
	}

	const URecallGameplayEffect* EffectCDO = Instance.EffectClass.GetDefaultObject();

	if (!EffectCDO->GameplayTagsToAdd.IsEmpty())
	{
		if (auto* GameplayTagFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
		{
			GameplayTagFragmentPtr->GameplayTagCountMap.RemoveTags(EffectCDO->GameplayTagsToAdd);
		}
	}

	if (!Instance.GameplayCueTags.IsEmpty())
	{
		if (auto* GameplayTagFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallGameplayTagFragment>())
		{
			for (const FGameplayTag& CueTag : Instance.GameplayCueTags)
			{
				GameplayTagFragmentPtr->GameplayTagCountMap.RemoveTag(CueTag);
			}
		}
	}
	
	if (!EffectCDO->AttributeMods.IsEmpty())
	{
		if (auto* AttributeFragmentPtr = OwnerView.GetFragmentDataPtr<FRecallAttributeFragment>())
		{
			for (const uint32 AttributeModHandle : Instance.AttributeModHandles)
			{
				AttributeFragmentPtr->AttributeSet.RemoveModByHandle(AttributeModHandle);
			}
		}
	}
}

TArray<TSubclassOf<URecallGameplayEffect>> FRecallGameplayEffectContainer::GetEffectClasses() const
{
	TArray<TSubclassOf<URecallGameplayEffect>> Results;
	Results.Reserve(EffectInstanceMap.Num());

	for (const TTuple<FRecallGameplayEffectHandle, FRecallGameplayEffectInstance>& EffectInstance : EffectInstanceMap)
	{
		Results.Add(EffectInstance.Value.EffectClass);
	}

	return Results;
}

TArray<FRecallGameplayEffectHandle> FRecallGameplayEffectContainer::GetAllEffectHandles(
	const TSubclassOf<URecallGameplayEffect>& Effect, const FMassEntityHandle& EffectOwnerEntity) const
{
	TArray<FRecallGameplayEffectHandle> Results;

	for (const TTuple<FRecallGameplayEffectHandle, FRecallGameplayEffectInstance>& EffectInstance : EffectInstanceMap)
	{
		if (EffectInstance.Value.EffectClass != Effect)
		{
			continue;
		}

		if (EffectOwnerEntity.IsSet() && EffectInstance.Value.OwnerEntity != EffectOwnerEntity)
		{
			continue;
		}

		Results.Add(EffectInstance.Key);
	}

	return Results;
}

void FRecallGameplayEffectContainer::TickEffects(const FRecallGameplayEffectContext& Context,
	float DeltaTime, ERecallGameplayEffectTickSource Source)
{	
	for (auto It = EffectInstanceMap.CreateIterator(); It; ++It)
	{
		if (TickEffect_Internal(DeltaTime, Source, It.Value()))
		{
			RemoveEffect_Internal(Context, It.Value());
			It.RemoveCurrent();
		}
	}	
}

bool FRecallGameplayEffectContainer::TickEffect_Internal(float DeltaTime, ERecallGameplayEffectTickSource Source,
	FRecallGameplayEffectInstance& Instance)
{
	const URecallGameplayEffect* EffectCDO = Instance.EffectClass.GetDefaultObject();
	if (!ensure(EffectCDO) || EffectCDO->DurationPolicy != ERecallGameplayEffectDurationType::HasDuration)
	{
		return true;
	}

	auto ShouldTickFromSource = [Source](const URecallGameplayEffect* EffectCDO)
	{
		switch (EffectCDO->TickRule)
		{
		case ERecallGameplayEffectTickRule::Default:
			return Source == ERecallGameplayEffectTickSource::Default;

		case ERecallGameplayEffectTickRule::Custom:
			return Source == ERecallGameplayEffectTickSource::Custom;

		default:
			unimplemented();
			return false;
		}
	};

	if (!ShouldTickFromSource(EffectCDO))
	{
		return false;
	}

	Instance.TimeSeconds += DeltaTime;

	return Instance.TimeSeconds >= EffectCDO->Duration && !FMath::IsNearlyZero(EffectCDO->Duration);
}
