// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGameplayEffectProcessors.h"

#include "Gameplay/RecallGameplayEffectContainer.h"
#include "MassExecutionContext.h"
#include "Simulation/GameplayEffect/RecallGameplayEffectFragments.h"

//----------------------------------------------------------------------//
// URecallGameplayEffectInitializer
//----------------------------------------------------------------------//
URecallGameplayEffectInitializer::URecallGameplayEffectInitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallGameplayEffectFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void URecallGameplayEffectInitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGameplayEffectInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGameplayEffectFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallGameplayEffectConstSharedFragment>();
}

void URecallGameplayEffectInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const FRecallGameplayEffectConstSharedFragment& SharedFragment =
			Context.GetConstSharedFragment<FRecallGameplayEffectConstSharedFragment>();
		const TArrayView<FRecallGameplayEffectFragment> GameplayEffectList = Context.GetMutableFragmentView<FRecallGameplayEffectFragment>();
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			
			FRecallGameplayEffectFragment& GameplayEffectFragment = GameplayEffectList[EntityIndex];
			GameplayEffectFragment.Container.Initialize(Entity);

			const FRecallGameplayEffectContext EffectContext{EntityManager, Entity};
			GameplayEffectFragment.Container.AddEffects(EffectContext, SharedFragment.DefaultEffects);
		}
	});
}

//----------------------------------------------------------------------//
// URecallGameplayEffectTickProcessor
//----------------------------------------------------------------------//
URecallGameplayEffectTickProcessor::URecallGameplayEffectTickProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void URecallGameplayEffectTickProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallGameplayEffectTickProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallGameplayEffectFragment>(EMassFragmentAccess::ReadWrite);
}

void URecallGameplayEffectTickProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_GameplayEffectTick_Execute);
	
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		
		const TArrayView<FRecallGameplayEffectFragment> GameplayEffectList = Context.GetMutableFragmentView<FRecallGameplayEffectFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			const FRecallGameplayEffectContext EffectContext{ EntityManager, Entity };
			
			FRecallGameplayEffectFragment& GameplayEffectFragment = GameplayEffectList[EntityIndex];
			GameplayEffectFragment.Container.TickEffects(EffectContext,
				DeltaTime, ERecallGameplayEffectTickSource::Default);
		}
	});
}
